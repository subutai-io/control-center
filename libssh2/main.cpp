#include <stdint.h>
#include <iostream>
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

#include "LibsshErrors.h"

struct CSshInitializer {
  int result;
  CSshInitializer() {
    result = libssh2_init(0);
  }
  ~CSshInitializer() {
    libssh2_exit();
  }
};
////////////////////////////////////////////////////////////////////////////

const char* run_libssh2_error_to_str(run_libssh2_error_t err) {
  static const char* rle_errors[] = {
    "SUCCESS", "WRONG_ARGUMENTS_COUNT", "WSA_STARTUP",
    "LIBSSH2_INIT", "INET_ADDR", "CONNECTION_TIMEOUT",
    "CONNECTION_ERROR", "LIBSSH2_SESSION_INIT", "SESSION_HANDSHAKE",
    "SSH_AUTHENTICATION", "LIBSSH2_CHANNEL_OPEN", "LIBSSH2_CHANNEL_EXEC",
    "LIBSSH2_EXIT_CODE_NOT_NULL"
  };
  return rle_errors[err];
}
////////////////////////////////////////////////////////////////////////////

int wait_ssh_socket_event(int socket_fd, LIBSSH2_SESSION *session) {
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

int wait_socket_connected(int socket_fd, int timeout_sec) {
  struct timeval timeout;
  fd_set fd;
  timeout.tv_sec = timeout_sec;
  timeout.tv_usec = 0;
  FD_ZERO(&fd);
  FD_SET(socket_fd, &fd);
  return select(socket_fd + 1, NULL, &fd, NULL, &timeout);
}
////////////////////////////////////////////////////////////////////////////

int run_ssh_command(const char* str_host, 
                    uint16_t port,
                    const char* str_user,
                    const char* str_pass,
                    const char* str_cmd,
                    int conn_timeout) {
  int rc = 0;
  struct sockaddr_in sin;
  unsigned long ul_host_addr = 0;

  int exitcode = 0;
  char *exitsignal = (char*)"none";

#ifndef _WIN32
  int sock;
  ul_host_addr = inet_addr(str_host);
  //todo check here
#else
  SOCKET sock;
  if (InetPtonA(AF_INET, str_host, &ul_host_addr) != 1) {
    return RLE_INET_ADDR;
  }
#endif
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = ul_host_addr;

  //todo in parallel thread with timeout
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
    while ((rc = libssh2_userauth_password(session, str_user, str_pass)) == LIBSSH2_ERROR_EAGAIN)
      ; //wait

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
      int lrc;
      do {
        char buffer[0x100];
        lrc = libssh2_channel_read(channel, buffer, sizeof(buffer));

        if (lrc > 0) {
//#ifdef LOG_STD_OUT
          std::string str(buffer, lrc);
          std::cout << str << std::endl;
//#endif
        }
        else {
          if (lrc != LIBSSH2_ERROR_EAGAIN) {
          }
        }
      } while (lrc > 0);

      /* this is due to blocking that would occur otherwise so we loop on
      this condition */
      if (lrc == LIBSSH2_ERROR_EAGAIN) {
        wait_ssh_socket_event(sock, session);
      }
      else {
        break;
      }
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

  return exitcode;// ? RLE_LIBSSH2_EXIT_CODE_NOT_NULL : RLE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  if (argc != 6) {
    std::cout << "Provide 5 parameters in this order : host_ip, port, username, password, command_to_run";
    return RLE_WRONG_ARGUMENTS_COUNT;
  }
  /*TODO implement getopt on windows and use getopt() everywhere*/
  const char* str_host = argv[1];
  const char* str_port = argv[2];
  const char* str_user = argv[3];
  const char* str_pass = argv[4];
  const char* str_cmd = argv[5];

#ifdef WIN32  
  WSADATA wsadata;
  if (int err = WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
    std::cout << "WSAStartup failed with error: " << err << std::endl;
    return RLE_WSA_STARTUP;
  }
#endif

  CSshInitializer libssh2_init;
  int rc = libssh2_init.result;
  if (rc) {
    std::cout << "libssh2_init error : " << rc << std::endl;
    return RLE_LIBSSH2_INIT;
  }

  int port = std::atoi(str_port);
  port = port ? port : 22;
  rc = run_ssh_command(str_host, port, str_user, str_pass, str_cmd, 10);

#ifdef _DEBUG
  system("pause");
#endif
  std::cout << rc << std::endl;
  return rc;
}
////////////////////////////////////////////////////////////////////////////
