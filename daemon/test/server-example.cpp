#include "cuda_commands.h"
#include "config_reader.h"
#include "server.h"

#define SOCKET_PATH "/tmp/server-test"

#define CONFIG_FILE "daemon.conf"

using namespace cuda_daemon;

cuda_daemon::command_base_t *create_ack() {
  cuda_daemon::command_base_t *response = (cuda_daemon::command_base_t *)malloc(sizeof(cuda_daemon::command_base_t));
  cuda_daemon::init_ack_command(*response);
  return response;
}

Server::message_result_t handle_hello_command(int id, const hello_command_t *cmd, Server &server) {
  printf("%s\n", cmd->message);
  server.send_on_socket(id, {create_ack(), sizeof(cuda_daemon::command_base_t)});
  return {Server::MessageListenerExitCode::OK, sizeof(hello_command_t), 0};
}

Server::message_result_t handle_end_command(int id, const command_base_t *cmd, Server &server) {
  (void)(cmd);
  server.send_on_socket(id, {create_ack(), sizeof(cuda_daemon::command_base_t)});
  server.stop();
  return {Server::MessageListenerExitCode::OK, sizeof(command_base_t), 0};
}

Server::message_result_t handle_variable_length_command(int id, const variable_length_command_t *cmd, Server &server) {
  server.send_on_socket(id, {create_ack(), sizeof(cuda_daemon::command_base_t)});
  return {Server::MessageListenerExitCode::OK, sizeof(variable_length_command_t), cmd->size}; // Ask to start reading size amount of bytes as plain data and return it via the data_listener
}

Server::message_result_t handle_command(int id, Server::message_t msg, Server &server) {
  if (msg.size < sizeof(command_base_t)) {
    return {Server::MessageListenerExitCode::INSUFFICIENT_DATA, 0, 0}; // Need to read more data to determine a command
  }
  command_base_t *base = (command_base_t *)msg.buf;
  switch (base->cmd) {
    case HELLO:
      if (msg.size >= sizeof(hello_command_t)) {
        return handle_hello_command(id, (hello_command_t *)msg.buf, server);
      }
      return {Server::MessageListenerExitCode::INSUFFICIENT_DATA, 0, 0};
      break;
    case END:
      if (msg.size >= sizeof(command_base_t)) {
        return handle_end_command(id, (command_base_t *)msg.buf, server);
      }
      return {Server::MessageListenerExitCode::INSUFFICIENT_DATA, 0, 0};
      break;
    case VARIABLE:
      if (msg.size >= sizeof(variable_length_command_t)) {
        return handle_variable_length_command(id, (variable_length_command_t *)msg.buf, server);
      }
      return {Server::MessageListenerExitCode::INSUFFICIENT_DATA, 0, 0};
      break;
    default:
      printf("handle: Unknown command\n");
      return {Server::MessageListenerExitCode::UNKNOWN_MESSAGE, 0, 0};
      break;
  }
}

Server::DataListenerExitCode data_listener(int id, Server::packet_t packet, Server &server) {
  printf("Data size: %li\n", packet.extra_data.size);
  printf("Content:\n%.*s\n", (int)packet.extra_data.size, (char *)packet.extra_data.buf);
  free(packet.extra_data.buf);
  free(packet.msg.buf);
  server.send_on_socket(id, {create_ack(), sizeof(cuda_daemon::command_base_t)});
  return Server::DataListenerExitCode::OK;
}

int main(int argc, char const *argv[]) {
  daemon_config_t config;
  auto config_res = ConfigReader::read_config(CONFIG_FILE, config);

  auto &logger = Logger::get_instance();

  if (config_res != ConfigReader::ExitCode::OK) {
    if (config_res == ConfigReader::ExitCode::CANNOT_OPEN_FILE) {
      logger.critical("Cannot open config file");
    } else {
      logger.critical("Error parsing config file");
    }
    return -1;
  }

  Logger::set_level(config.log_level);

  Server server(SOCKET_PATH, 10, handle_command, data_listener);
  server.initialize();

  logger.info("Server initialized, starting loop...");

  server.start();

  return 0;
}
