#include <string>
#include <map>
#include "types.h"

namespace dynamic_compiler {

struct base_arch_config {
  std::string compiler_path;
  bool libclang;
};

struct dynamic_compiler_config {
    int expiration_time;
    std::map<hhal::Unit, base_arch_config> base_arch_configs;
};

class ConfigReader {

public:
  enum class ExitCode {
    OK,               // Success
    CANNOT_OPEN_FILE, // Cannot open config file
    ERROR             // Generic error
  };

  static ExitCode read_config(std::string path, dynamic_compiler_config &config);

private:
  ConfigReader();
};

} // namespace daemon
