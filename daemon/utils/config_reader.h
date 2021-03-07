#include "utils/logger.h"
#include <string>

namespace hhal_daemon {

struct daemon_config_t {
  Logger::Level log_level;
};

class ConfigReader {

public:
  enum class ExitCode {
    OK,               // Success
    CANNOT_OPEN_FILE, // Cannot open config file
    ERROR             // Generic error
  };

  static ExitCode read_config(std::string path, daemon_config_t &config);

private:
  ConfigReader();
};

} // namespace daemon
