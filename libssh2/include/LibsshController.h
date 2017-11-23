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

/**
 * @brief The CLibsshController class is wrapper for libssh2 functionality
 */
class CLibsshController {

private:
  struct CSshInitializer {
    int result;
    CSshInitializer();
    ~CSshInitializer();
  };

  static CSshInitializer m_initializer;

public:
  static const char *run_libssh2_error_to_str(run_libssh2_error_t err);

  /**
   * @brief Run ssh command with password authorization
   * @param host
   * @param port
   * @param user
   * @param pass
   * @param cmd
   * @param conn_timeout
   * @param lst_out - command output
   * @return run_libssh2_error_t
   */
  static int run_ssh_command_pass_auth(const char *host,
                                       uint16_t port,
                                       const char *user,
                                       const char *pass,
                                       const char *cmd,
                                       int conn_timeout,
                                       std::vector<std::string>& lst_out);

  /**
   * @brief Run ssh command with key authorization
   * @param host
   * @param port
   * @param pub_file - public key path
   * @param pr_file - private key path
   * @param passphrase
   * @param cmd
   * @param conn_timeout
   * @param lst_out - command output
   * @return run_libssh2_error_t
   */
  static int run_ssh_command_key_auth(const char *host,
                                      uint16_t port,
                                      const char* pub_file,
                                      const char* pr_file, const char *passphrase,
                                      const char* cmd,
                                      int conn_timeout,
                                      std::vector<std::string>& lst_out);
};

#endif // LIBSSHCONTROLLER_H
