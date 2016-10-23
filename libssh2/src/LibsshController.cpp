#include "libssh2/include/LibsshController.h"

#include <stdint.h>
#include <libssh2.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#define SOCKET_ERROR (-1)
#endif

#include <string>
#include <stdlib.h>

#include "libssh2/include/LibsshController.h"
#include "ApplicationLog.h"

CLibsshController::CSshInitializer CLibsshController::m_initializer;

/*!
 * \brief Argument for login/password based authorization
 */
struct rsc_user_pass_arg_t {
  const char* user;
  const char* pass;
};

/*!
 * \brief Argument for ssh key file based authorization
 */
struct rsc_pub_key_arg_t {
  const char* pub_file;
  const char* privae_file;
  const char* passphrase;
};

int wait_socket_connected(int socket_fd, int timeout_sec);
int user_pass_authentication(LIBSSH2_SESSION *session, const void* rsc_user_pass_arg);
int key_pub_authentication(LIBSSH2_SESSION *session, const void* rsc_pub_key_arg);

int run_ssh_command_internal(const char* str_host,
                             uint16_t port,
                             const char* str_cmd,
                             int conn_timeout,
                             std::vector<std::string> &lst_out,
                             int (*pf_auth)(LIBSSH2_SESSION*, const void *),
                             void *pf_auth_arg);

CLibsshController::CSshInitializer::CSshInitializer()
{
  do {
#ifdef WIN32
    WSADATA wsadata;
    if (int err = WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
      CApplicationLog::Instance()->LogError("WSAStartup failed with error: %d", err);
      result = RLE_WSA_STARTUP;
      break;
    }
#endif
    result = libssh2_init(0);
  } while (0);
}
////////////////////////////////////////////////////////////////////////////

CLibsshController::CSshInitializer::~CSshInitializer() {
  libssh2_exit();
}
////////////////////////////////////////////////////////////////////////////

const char*
CLibsshController::run_libssh2_error_to_str(run_libssh2_error_t err) {
  if (err < RLE_SUCCESS) return "exit code not null";
  int index = (int)err - (int)RLE_SUCCESS;
  static const char* rle_errors[] = {
    "SUCCESS", "WRONG_ARGUMENTS_COUNT", "WSA_STARTUP",
    "LIBSSH2_INIT", "INET_ADDR", "CONNECTION_TIMEOUT",
    "CONNECTION_ERROR", "LIBSSH2_SESSION_INIT", "SESSION_HANDSHAKE",
    "SSH_AUTHENTICATION", "LIBSSH2_CHANNEL_OPEN", "LIBSSH2_CHANNEL_EXEC",
    "LIBSSH2_EXIT_CODE_NOT_NULL"
  };
  return rle_errors[index];
}
////////////////////////////////////////////////////////////////////////////

int
wait_ssh_socket_event(int socket_fd,
                      LIBSSH2_SESSION *session) {
  struct timeval timeout;
  int rc;
  fd_set fd;
  fd_set *writefd = NULL;
  fd_set *readfd = NULL;
  int dir;

  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  FD_ZERO(&fd);
  FD_SET(socket_fd, &fd);

  /* now make sure we wait in the correct direction */
  dir = libssh2_session_block_directions(session);
  if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
    readfd = &fd;

  if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
    writefd = &fd;

  rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
  return rc;
}
////////////////////////////////////////////////////////////////////////////

int
user_pass_authentication(LIBSSH2_SESSION *session, const void *rsc_user_pass_arg) {
  rsc_user_pass_arg_t* arg = (rsc_user_pass_arg_t*)rsc_user_pass_arg;
  return libssh2_userauth_password(session, arg->user, arg->pass);
}
////////////////////////////////////////////////////////////////////////////

int
key_pub_authentication(LIBSSH2_SESSION *session, const void *rsc_pub_key_arg) {
  UNUSED_ARG(session);
  UNUSED_ARG(rsc_pub_key_arg);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int
wait_socket_connected(int socket_fd, int timeout_sec) {
  struct timeval timeout;
  fd_set fd;
  timeout.tv_sec = timeout_sec;
  timeout.tv_usec = 0;
  FD_ZERO(&fd);
  FD_SET(socket_fd, &fd);
  return select(socket_fd + 1, NULL, &fd, NULL, &timeout);
}
////////////////////////////////////////////////////////////////////////////

int
run_ssh_command_internal(const char *str_host,
                         uint16_t port,
                         const char *str_cmd,
                         int conn_timeout,
                         std::vector<std::string> &lst_out,
                         int (*pf_auth)(LIBSSH2_SESSION*, const void *),
                         void *pf_auth_arg) {
  int rc = 0;
  struct sockaddr_in sin;
  unsigned long ul_host_addr = 0;
  int exitcode = 0;
  char *exitsignal = (char*)"none";


#ifndef _WIN32
  int sock;
  ul_host_addr = inet_addr(str_host);
#else
  SOCKET sock;
  if (InetPtonA(AF_INET, str_host, &ul_host_addr) != 1) {
    return RLE_INET_ADDR;
  }
#endif
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = ul_host_addr;
  sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
  u_long mode = 1;
  ioctlsocket(sock, FIONBIO, &mode);
#else
  int flags = fcntl(sock, F_GETFL, 0);
  flags |= O_NONBLOCK;
#endif
  connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in));
  rc = wait_socket_connected(sock, conn_timeout);

  if (rc == 0) {
    return RLE_CONNECTION_TIMEOUT;
  }
  else if (rc == SOCKET_ERROR) {
    return RLE_CONNECTION_ERROR;
  }

  LIBSSH2_SESSION *session = libssh2_session_init();
  if (!session) {
    return RLE_LIBSSH2_SESSION_INIT;
  }

  while ((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN)
    ; //wait

  if (rc) {
    return RLE_SESSION_HANDSHAKE;
  }

  do {
    while ((rc = pf_auth(session, pf_auth_arg)) == LIBSSH2_ERROR_EAGAIN)
      ;

    if (rc) {
      return RLE_SSH_AUTHENTICATION;
    }

    LIBSSH2_CHANNEL *channel;
    while ((channel = libssh2_channel_open_session(session)) == NULL &&
           libssh2_session_last_error(session, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN) {
      wait_ssh_socket_event(sock, session);
    }

    if (channel == NULL) {
      return RLE_LIBSSH2_CHANNEL_OPEN;
    }

    while ((rc = libssh2_channel_exec(channel, str_cmd)) ==
           LIBSSH2_ERROR_EAGAIN) {
      wait_ssh_socket_event(sock, session);
    }

    if (rc != 0) {
      return RLE_LIBSSH2_CHANNEL_EXEC;
    }

    for (;;) {
      /* loop until we block */
      int f, l, r;
      static char buffer[0x100] = {0};
      f = l = 0;
      r = -1;

      while ((r = libssh2_channel_read(channel, &buffer[l], sizeof(buffer) - l - 1)) > 0) {
        buffer[l+r] = 0;

        for (; buffer[l]; ++l) {
          if (buffer[l] != '\n') continue;
          lst_out.push_back(std::string(&buffer[f], l-f));
          f = l+1;
        }

        memcpy(buffer, &buffer[f], l-f);
        l = l-f;
        f = 0;
      }

      /* this is due to blocking that would occur otherwise so we loop on
      this condition */
      if (r != LIBSSH2_ERROR_EAGAIN)
        break;
      wait_ssh_socket_event(sock, session);
    }

    exitcode = 127;
    while ((rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN)
      wait_ssh_socket_event(sock, session);

    if (rc == 0) {
      exitcode = libssh2_channel_get_exit_status(channel);
      libssh2_channel_get_exit_signal(channel, &exitsignal,
                                      NULL, NULL, NULL, NULL, NULL);
    }

    libssh2_channel_free(channel);
    channel = NULL;
  } while (0);

  libssh2_session_disconnect(session,
                             "Normal Shutdown, Thank you for playing");
  libssh2_session_free(session);

#ifdef _WIN32
  closesocket(sock);
#else
  close(sock);
#endif

  return exitcode;
}
////////////////////////////////////////////////////////////////////////////

int
CLibsshController::run_ssh_command_pass_auth(const char* host,
                                             uint16_t port,
                                             const char* user,
                                             const char* pass,
                                             const char* cmd,
                                             int conn_timeout,
                                             std::vector<std::string> &lst_out) {
  if (m_initializer.result != 0) return RLE_LIBSSH2_INIT;
  rsc_user_pass_arg_t arg;
  memset(&arg, 0, sizeof(rsc_user_pass_arg_t));
  arg.user = user;
  arg.pass = pass;
  return run_ssh_command_internal(host, port, cmd, conn_timeout,
                                  lst_out, user_pass_authentication, &arg);
}
////////////////////////////////////////////////////////////////////////////

int
CLibsshController::run_ssh_command_key_auth(const char *host,
                                            uint16_t port,
                                            const char *pub_file,
                                            const char *pr_file,
                                            const char *passphrase,
                                            const char *cmd,
                                            int conn_timeout,
                                            std::vector<std::string> &lst_out) {
  if (m_initializer.result != 0) return RLE_LIBSSH2_INIT;
  rsc_pub_key_arg_t arg;
  memset(&arg, 0, sizeof(rsc_pub_key_arg_t));
  arg.passphrase = passphrase;
  arg.privae_file = pr_file;
  arg.pub_file = pub_file;
  return run_ssh_command_internal(host, port, cmd, conn_timeout,
                                  lst_out, key_pub_authentication, &arg);
}
////////////////////////////////////////////////////////////////////////////
