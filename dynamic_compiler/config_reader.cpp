#include "dynamic_compiler/config_reader.h"
#include "dynamic_compiler/inih/INIReader.h"

namespace dynamic_compiler {

ConfigReader::ExitCode ConfigReader::read_config(std::string path, dynamic_compiler_config &config) {
    auto reader = INIReader(path);

    int expiration = reader.GetInteger("compiler", "expiration", 259200); // 3 days by default

    base_arch_config gn_config;
    gn_config.compiler_path = reader.Get("GN", "path", "");
    gn_config.libclang = reader.GetBoolean("GN", "libclang", true);

    printf("Compiler: Reading config file %s", path.c_str());

    /*
    printf("expiration time: %d\n", expiration);
    printf("gn compiler path: %s\n", gn_config.compiler_path.c_str());
    printf("gn libclang: %d\n", gn_config.libclang);
    */

    config.expiration_time = expiration;
    config.base_arch_configs[hhal::Unit::GN] = gn_config;

    return ExitCode::OK;
}

} // namespace dynamic_compiler
