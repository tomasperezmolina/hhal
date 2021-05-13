#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <glob.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

#ifdef LLVM_ENABLED
#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Job.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Target/TargetMachine.h"

#include "dynamic_compiler/LLVMInstanceManager.h"
#endif

#include "cuda_compiler.h"

#include "dynamic_compiler/compiler.h"
#include "dynamic_compiler/mango_gen_kernel_entry.h"

namespace dynamic_compiler {

    bool exists(const std::string &name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

    void delete_unused_files(const std::string& pattern, int expiration) {
        std::time_t current_time = std::time(NULL);
        glob_t glob_result;
        int res = glob(pattern.c_str(), GLOB_TILDE | GLOB_MARK, NULL, &glob_result);
        if (res == 0) {
            for(unsigned int i = 0; i < glob_result.gl_pathc; ++i){
                char* filename = glob_result.gl_pathv[i];
                //printf("Compiler: found cached kernel %s\n", filename);
                struct stat cached_bin;
                int stat_res = stat(filename, &cached_bin);
                if (stat_res == 0) {
                    long file_age = current_time - cached_bin.st_atim.tv_sec;
                    if (file_age > expiration) {
                        printf("Compiler: deleting expired kernel (%ld seconds)\n", file_age);
                        remove(filename);
                    }
                }
            }
        }
        globfree(&glob_result);
    }

    inline const char *unit_to_string(hhal::Unit unit) {
        switch(unit) {
            case hhal::Unit::GN:        return "GN";
            case hhal::Unit::NVIDIA:    return "NVIDIA";
            default:                    return "";
        }
    }

    Compiler::Compiler() {
        ConfigReader::ExitCode config_res = ConfigReader::read_config(DYNAMIC_COMPILER_CONFIG, config);
    }

    const std::string Compiler::get_binary(const std::string source, hhal::Unit arch) {
        delete_unused_files(KERNEL_CACHE_BASE_DIR "/*", expiration_time);

        auto error_str = "";

        auto filename = source.substr(source.find_last_of('/') + 1);
        auto filename_no_ext = filename.substr(0, filename.find_last_of('.'));
        auto bin_path = KERNEL_CACHE_BASE_DIR "/" + filename_no_ext;

        bool should_compile = true;

        struct stat binary_result;
        struct stat source_result;
        if(stat(source.c_str(), &source_result) == 0) {
            auto source_mod_time = source_result.st_mtime;
            if (stat(bin_path.c_str(), &binary_result) == 0) {
                auto binary_mod_time = binary_result.st_mtime;
                should_compile = binary_mod_time < source_mod_time;
            }
        } else {
            // Source file was not located
            printf("Compiler: Unable to open kernel file [%s]\n", source.c_str());
            return error_str;
        }

        if (should_compile) {
            printf("Compiler: Kernel file [%s] compiling...\n", source.c_str());

            switch(arch) {
                case hhal::Unit::NVIDIA: 
                {
                    cuda_compiler::CudaCompiler cuda_compiler;
                    char *ptx;
                    cuda_compiler.compile_to_ptx(source.c_str(), &ptx);
                    cuda_compiler.save_ptx_to_file(ptx, bin_path.c_str());
                    delete[] ptx;

                    break;
                }
                case hhal::Unit::GN: 
                {
                    std::string entry_path = bin_path + "_entry.c";
                    bool generated_entry = generate_entrypoint(source, entry_path, arch);
                    printf("Compiler: Generated entrypoint %d\n", generated_entry);

                    bool success;
                    if(generated_entry)
                        success = gn_generate_bin({source, entry_path, "-I/opt/mango/usr/include/libmango/", "-L/opt/mango/usr/lib", "-pthread", "-lmango-dev-gn"}, bin_path, arch);
                    else
                        success = gn_generate_bin({source, "-I/opt/mango/usr/include/libmango/", "-L/opt/mango/usr/lib", "-pthread", "-lmango-dev-gn"}, bin_path, arch);

                    if (!success) return error_str;
                    break;
                }
                default:
                    return error_str;
            }
        } else {
            printf("Compiler: Skipping compilation. Compiled binary for kernel file\n");
            printf("Compiler: %s\n", source.c_str());
            printf("Compiler: already present at [%s]\n", bin_path.c_str());
        }

        return bin_path;
    }

    const std::string save_to_file(const std::string kernel_string) {
        auto error_str = "";
        std::hash<std::string> hasher;
        auto kernel_hash = hasher(kernel_string);

        std::stringstream stream;
        stream << std::hex << kernel_hash;
        std::string hash_string( stream.str() );

        auto path = KERNEL_CACHE_BASE_DIR "/kernel-" + hash_string + ".c";

        struct stat existing_kernel;
        if (stat(path.c_str(), &existing_kernel) != 0) {
            printf("Compiler: Generating kernel source file from string with filename [%s]\n", path.c_str());
            std::ofstream kernel_file(path);
            if (!kernel_file) {
                printf("Compiler: Cannot create file to save kernel string\n");
                return error_str;
            }
            kernel_file << kernel_string;
            kernel_file.close();
        } else {
            printf("Compiler: Kernel source file already generated for kernel string [%s], skipping generation\n", path.c_str());
        }

        return path;
    }

    bool Compiler::gn_generate_bin(const std::vector<std::string> &cmds, const std::string outputFilename, hhal::Unit arch) const {
        const std::string outputArgument = "-o" + outputFilename;

        base_arch_config arch_config = config.base_arch_configs.at(arch);

        if(arch_config.libclang) {
#ifdef LLVM_ENABLED
            auto _llvmManager = LLVMInstanceManager::getInstance();

            auto _diagnosticOptions = new clang::DiagnosticOptions();
            auto _diagnosticIDs = new clang::DiagnosticIDs();
            auto _diagEngine = new clang::DiagnosticsEngine(_diagnosticIDs, _diagnosticOptions);

            std::vector<const char *> cmd_str;
            cmd_str.push_back("clang");

            for (const auto& cmd : cmds) {
                cmd_str.push_back(cmd.c_str());
            }

            cmd_str.push_back(std::move(outputArgument).c_str());

            // log the command line string used to create this task (split into multiple lines if > 256 chars).
            std::vector<std::string> command_vector;
            std::stringstream ss;
            for (const auto& arg : cmd_str) {
                if(int(ss.tellp()) + strlen(arg) > 256) {
                    command_vector.push_back(ss.str());
                    ss.str("");
                }
                ss << arg << " ";
            }
            command_vector.push_back(ss.str());

            printf("Compiler: libclang executing\n");

            for (const auto& cmd : command_vector) {
                printf("%s\n", cmd.c_str());
            }

            clang::driver::Driver Driver(_llvmManager->getClangExePath(),
                                    _llvmManager->getDefaultTriple()->str(),
                                    *_diagEngine);
            Driver.setCheckInputsExist(false);
            Driver.CCPrintOptions = false;

            std::unique_ptr<clang::driver::Compilation> C(Driver.BuildCompilation(cmd_str));
            if (!C) {
                printf("Compiler: clang::driver::Compilation not created\n");
                return false;
            }

            llvm::SmallVector<std::pair<int, const clang::driver::Command*>,1> failCmd;
            const auto res = Driver.ExecuteCompilation(*C, failCmd);

            if (exists(outputFilename)) {
                return true;
            }

            printf("Compiler: Unknown error, unable to generate shared object. Driver error code %d\n", res);

            return false;
#else
            printf("Compiler: libclang not available, compiling with system call\n");
#endif
        }

        if(arch_config.compiler_path == "") {
            printf("Compiler: No path to %s compiler given\n", unit_to_string(arch));
            return false;
        }

        // log the command line string used to create this task (split into multiple lines if > 256 chars).
        std::vector<std::string> full_commands;
        full_commands.push_back(arch_config.compiler_path);
        full_commands.insert(full_commands.end(), cmds.begin(), cmds.end());
        full_commands.push_back(outputArgument);
        std::vector<std::string> command_vector;
        std::stringstream partial_stream;
        std::stringstream full_stream;
        for (const auto& arg : full_commands) {
            if(int(partial_stream.tellp()) + arg.size() > 256) {
                command_vector.push_back(partial_stream.str());
                partial_stream.str("");
            }
            partial_stream << arg << " ";
            full_stream << arg << " ";
        }
        command_vector.push_back(partial_stream.str());

        printf("Compiler: system executing\n");
        for (const auto& cmd : command_vector) {
            printf("%s\n", cmd.c_str());
        }

        int res = system(full_stream.str().c_str());

        if (res == 0) return true;
        else printf("Compiler: System call failed, result code = %d\n", res);

        return false;
    } 
 
}
