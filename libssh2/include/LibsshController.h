#ifndef LIBSSHCONTROLLER_H
#define LIBSSHCONTROLLER_H

#include <stdint.h>
#include <vector>
#include <string>
#include "LibsshErrors.h"

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
