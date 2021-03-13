#include "hhal_server.h"
#include "utils/config_reader.h"

using namespace hhal_daemon;

int main(int argc, char const *argv[]) { 
    auto &logger = Logger::get_instance();

    daemon_config_t config;
    auto config_res = ConfigReader::read_config(DAEMON_CONFIG, config);

    if (config_res != ConfigReader::ExitCode::OK) {
        if (config_res == ConfigReader::ExitCode::CANNOT_OPEN_FILE) {
            logger.critical("Cannot open config file");
        } else {
            logger.critical("Error parsing config file");
        }
        return -1;
    }

    std::string socket_path;
    if(argc > 1) {
        socket_path = argv[1];
    } else {
        socket_path = config.daemon_path;
    }

    logger.info("LOG LEVEL: {}", Logger::level_to_string(config.log_level));

    Logger::set_level(config.log_level);

    logger.info("Server initialized, starting loop...");

    HHALServer hhal_server(socket_path);
}
