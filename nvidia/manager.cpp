#include <fstream>
#include <algorithm>

#include "nvidia/manager.h"
#include "kernel_arguments.h"

namespace hhal {

    NvidiaManagerExitCode NvidiaManager::assign_kernel(nvidia_kernel *info) {
        kernel_info[info->id] = *info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::assign_buffer(nvidia_buffer *info) {
        buffer_info[info->id] = *info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::assign_event(nvidia_event *info) {
        event_info[info->id] = *info;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::kernel_write(int kernel_id, std::string image_path) {
        nvidia_kernel &info = kernel_info[kernel_id];

        std::ifstream input_file(image_path, std::ifstream::in | std::ifstream::ate);

        if (!input_file.is_open()) {
            return NvidiaManagerExitCode::ERROR;
        }

        size_t input_size = (size_t) input_file.tellg();
        size_t buffer_size = input_size + 1;
        char *ptx = new char[buffer_size];

        input_file.seekg(0, std::ifstream::beg);
        input_file.read(ptx, input_size);
        input_file.close();
        ptx[input_size] = '\0';

        CudaApiExitCode err = cuda_api.write_kernel(info.mem_id, ptx, buffer_size);

        delete[] ptx;

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        // For now we assume that the function name is equal to the filename
        std::string function_name = image_path;

        // Remove directory if present.
        const size_t last_slash_idx = function_name.find_last_of("/");
        if (std::string::npos != last_slash_idx) {
            function_name.erase(0, last_slash_idx + 1);
        }

        // Remove extension if present.
        const size_t period_idx = function_name.rfind('.');
        if (std::string::npos != period_idx) {
            function_name.erase(period_idx);
        }
        info.function_name = function_name;

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::kernel_start(int kernel_id, const Arguments &arguments) {
        auto &args = arguments.get_args();
        int arg_count = args.size();
        size_t arg_array_size = 0;
        for(auto &arg: args) {
            switch (arg.type) {
                case ArgumentType::BUFFER:
                    arg_array_size += sizeof(cuda_manager::BufferArg);
                    break;
                case ArgumentType::EVENT:
                    printf("Events not supported yet\n");
                    return NvidiaManagerExitCode::ERROR; 
                    break;
                case ArgumentType::SCALAR:
                    if (arg.scalar.type == ScalarType::FLOAT && arg.scalar.size == sizeof(float)) {
                        arg_array_size += sizeof(cuda_manager::ValueArg);
                    }
                    else {
                        printf("Only floats are supported at the moment\n");
                        return NvidiaManagerExitCode::ERROR; 
                    }
                    break;
                default:
                    printf("Unknown argument\n");
                    return NvidiaManagerExitCode::ERROR; 
            }
        }

        char *arg_array = (char*) malloc(arg_array_size);
        char *current_arg = arg_array;

        for(auto &arg: args) {
            switch (arg.type) {
                case ArgumentType::BUFFER: {
                    auto &b_info = buffer_info[arg.buffer.id];
                    auto &in_kernels = b_info.kernels_in;
                    bool is_in = std::find(in_kernels.begin(), in_kernels.end(), kernel_id) != in_kernels.end();
                    auto *arg_x = (cuda_manager::BufferArg *) current_arg;
                    *arg_x = {cuda_manager::BUFFER, b_info.id, is_in};
                    current_arg += sizeof(cuda_manager::BufferArg);
                    break;
                } 
                case ArgumentType::SCALAR: {
                    auto *arg_a = (cuda_manager::ValueArg *) current_arg;
                    *arg_a = {cuda_manager::VALUE, * (float*) arg.scalar.address};
                    current_arg += sizeof(cuda_manager::ValueArg);
                    arg_array_size += sizeof(cuda_manager::ValueArg);
                    break;
                }
                default:
                    printf("This should not happen\n");
                    return NvidiaManagerExitCode::ERROR;
            }
        }

        CudaApiExitCode err = cuda_api.launch_kernel(kernel_id, kernel_info[kernel_id].function_name.c_str(), arg_array, arg_count);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        // TODO a separate thread should set this to indicate the kernel is done, also the arguments should specify the kernel.
        write_sync_register(0, 1);
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::allocate_memory(int buffer_id) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.allocate_memory(info.mem_id, info.size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR; 
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::allocate_kernel(int kernel_id) {
        nvidia_kernel &info = kernel_info[kernel_id];

        CudaApiExitCode err = cuda_api.allocate_kernel(info.mem_id, info.size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::release_memory(int buffer_id) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.deallocate_memory(info.mem_id);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }
    
    NvidiaManagerExitCode NvidiaManager::release_kernel(int buffer_id) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.deallocate_kernel(info.mem_id);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::write_to_memory(int buffer_id, const void *source, size_t size) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.write_memory(info.mem_id, source, size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::read_from_memory(int buffer_id, void *dest, size_t size) {
        nvidia_buffer &info = buffer_info[buffer_id];

        CudaApiExitCode err = cuda_api.read_memory(info.mem_id, dest, size);

        if (err != OK) {
            return NvidiaManagerExitCode::ERROR;
        }

        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::write_sync_register(int event_id, uint8_t data) {
        this->data = data;
        return NvidiaManagerExitCode::OK;
    }

    NvidiaManagerExitCode NvidiaManager::read_sync_register(int event_id, uint8_t *data) {
        uint8_t res = this->data;
        this->data = 0;
        *data = res;
        return NvidiaManagerExitCode::OK;
    }
}
