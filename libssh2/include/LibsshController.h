#ifndef LIBSSHCONTROLLER_H
#define LIBSSHCONTROLLER_H

#include <stdint.h>
#include <vector>
#include <string>

typedef enum run_libssh2_error {
  RLE_SUCCESS = 1000,
  RLE_WRONG_ARGUMENTS_COUNT,
  RLE_WSA_STARTUP,
  RLE_LIBSSH2_INIT,
  RLE_INET_ADDR,
  RLE_CONNECTION_TIMEOUT,
  RLE_CONNECTION_ERROR,
  RLE_LIBSSH2_SESSION_INIT,
  RLE_SESSION_HANDSHAKE,
  RLE_SSH_AUTHENTICATION,
  RLE_LIBSSH2_CHANNEL_OPEN,
  RLE_LIBSSH2_CHANNEL_EXEC,
  RLE_LIBSSH2_EXIT_CODE_NOT_NULL
} run_libssh2_error_t;

class CLibsshController {

private:
  struct CSshInitializer {
    int result;
    CSshInitializer();
    ~CSshInitializer();
  };

  static CSshInitializer m_initializer;

  static int wait_socket_connected(int socket_fd, int timeout_sec);
public:
  static const char *run_libssh2_error_to_str(run_libssh2_error_t err);
  static int run_ssh_command(const char *str_host,
                             uint16_t port,
                             const char *str_user,
                             const char *str_pass,
                             const char *str_cmd,
                             int conn_timeout,
                             std::vector<std::string>& lst_out);
};

#endif // LIBSSHCONTROLLER_H
