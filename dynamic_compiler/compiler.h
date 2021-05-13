#ifndef DYNAMIC_COMPILER_H
#define DYNAMIC_COMPILER_H

#include <vector>
#include <string>
#include "dynamic_compiler/config_reader.h"

namespace dynamic_compiler {

    /*! \brief Save the kernel string to a file.
    * \param kernel_string Kernel string to save
    * \returns Path to generated file
    * \note If a file with the same contents already exists, the path to it is returned, instead of generating a new file or overwriting it.
    */
    const std::string save_to_file(const std::string kernel_string);

    /*! \class Compiler
    * \brief Hold the configuration for compiling for each architecture
    */
    class Compiler {
        public:
            Compiler();

            /*! \brief Get compiled binary based on a source file.
            * \param source The source file for the needed binary
            * \param type Unit type to compile the source file for
            * \returns Path to binary file
            * \note Compilation is only performed if there is no binary for the given source file that is newer than the source file.
            */
            const std::string get_binary(const std::string source, hhal::Unit arch);

        private:
            bool gn_generate_bin(const std::vector<std::string> &cmds, const std::string outputFilename, hhal::Unit arch) const;

            int expiration_time = 259200; // 3 days by default

            dynamic_compiler_config config;
    };
}

#endif /* DYNAMIC_COMPILER_H */
