#ifndef SYSTEMCALLWRAPPER_H
#define SYSTEMCALLWRAPPER_H

#include <stdint.h>
#include <QObject>
#include <QString>
#include <string>
#include <vector>

//give type for restart p2p
enum restart_p2p_type{
    UPDATED_P2P=0, //when p2p updated, stop and start
    STOPPED_P2P //when p2p is stopped, start
};

// if you are going to change this - change method
// CSystemCallWrapper::scwe_error_to_str(system_call_wrapper_error_t err)
enum system_call_wrapper_error_t {
  /*common errors*/
  SCWE_SUCCESS = 0,
  SCWE_SHELL_ERROR,
  SCWE_PIPE,
  SCWE_SET_HANDLE_INFO,
  SCWE_CREATE_PROCESS,

  /*p2p errors*/
  SCWE_P2P_IS_NOT_RUNNABLE,
  SCWE_CANT_JOIN_SWARM,
  SCWE_CONTAINER_IS_NOT_READY,
  SCWE_CANT_SEND_HANDSHAKE,

  /*ssh errors*/
  SCWE_SSH_LAUNCH_FAILED,
  SCWE_CANT_GET_RH_IP,
  SCWE_CANT_GENERATE_SSH_KEY,

  /*other errors*/
  SCWE_TIMEOUT,
  SCWE_WHICH_CALL_FAILED,
  SCWE_PROCESS_CRASHED,
  SCWE_LAST
};
////////////////////////////////////////////////////////////////////////////

struct system_call_res_t {
  system_call_wrapper_error_t res;
  QStringList out;
  int exit_code;
};
////////////////////////////////////////////////////////////////////////////

enum restart_service_error_t { RSE_SUCCESS, RSE_MANUAL };
////////////////////////////////////////////////////////////////////////////

class CSystemCallThreadWrapper : public QObject {
  Q_OBJECT
 private:
  system_call_res_t m_result;
  QString m_command;
  QStringList m_args;
  bool m_read_output;

 public:
  CSystemCallThreadWrapper(QObject *parent = 0)
      : QObject(parent), m_command(""), m_read_output(true) {}

  CSystemCallThreadWrapper(const QString &command, const QStringList &args,
                           bool arg_read_output, QObject *parent = 0)
      : QObject(parent),
        m_command(command),
        m_args(args),
        m_read_output(arg_read_output) {}

  system_call_res_t result() const { return m_result; }
  bool read_output() const { return m_read_output; }
  void abort() { emit finished(); }

 signals:
  void finished();
};
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This class contains methods and functions for receiving output of
 * system calls like "ls -la", "p2p version" etc.
 */
class CSystemCallWrapper {
  friend class CSystemCallThreadWrapper;

 private:
  static system_call_wrapper_error_t run_libssh2_command(
      const char *host, uint16_t port, const char *user, const char *pass,
      const char *cmd, int &exit_code, std::vector<std::string> &lst_output);

 public:
  static system_call_res_t ssystem_th(const QString &cmd,
                                      const QStringList &args, bool read_output,
                                      bool log,
                                      unsigned long timeout_msec = ULONG_MAX);

  static system_call_res_t ssystem(const QString &cmd, const QStringList &args,
                                   bool read_out, bool log,
                                   unsigned long timeout_msec = 30000);

  static bool is_in_swarm(const QString &hash);
  static std::vector<QString> p2p_show();
  static std::vector<std::pair<QString, QString>> p2p_show_interfaces();

  static system_call_wrapper_error_t join_to_p2p_swarm(const QString &hash,
                                                       const QString &key,
                                                       const QString &ip,
                                                       int swarm_base_interface_id);

  static system_call_wrapper_error_t leave_p2p_swarm(const QString &hash);
  static system_call_wrapper_error_t restart_p2p_service(int *res_code, restart_p2p_type type);

  static system_call_wrapper_error_t check_container_state(const QString &hash,
                                                           const QString &ip);

  static system_call_wrapper_error_t run_sshkey_in_terminal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &key);

  static system_call_wrapper_error_t run_sshpass_in_terminal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &pass);

  static system_call_wrapper_error_t run_x2goclient_session(const QString &session_id);


  static system_call_wrapper_error_t run_x2go(QString remote_ip,
                                              QString remote_port,
                                              QString remote_username);

  static system_call_wrapper_error_t generate_ssh_key(const QString &comment,
                                                      const QString &file_path);

  static system_call_wrapper_error_t is_rh_update_available(bool &available);
  static system_call_wrapper_error_t is_rh_management_update_available(
      bool &available);

  static system_call_wrapper_error_t is_peer_available(
      const QString &peer_fingerprint, int* exit_code);

  static system_call_wrapper_error_t run_rh_updater(const char *host,
                                                    uint16_t port,
                                                    const char *user,
                                                    const char *pass,
                                                    int &exit_code);

  static system_call_wrapper_error_t run_rh_management_updater(const char *host,
                                                               uint16_t port,
                                                               const char *user,
                                                               const char *pass,
                                                               int &exit_code);

  static system_call_wrapper_error_t get_rh_ip_via_libssh2(
      const char *host, uint16_t port, const char *user, const char *pass,
      int &exit_code, std::string &ip);

  static QString rh_version();
  static QString rhm_version();

  static system_call_wrapper_error_t p2p_version(QString &version);
  static system_call_wrapper_error_t p2p_status(QString &status);
  static bool p2p_daemon_check();
  static bool x2goclient_check();


  static system_call_wrapper_error_t which(const QString &prog, QString &path);
  static system_call_wrapper_error_t open(const QString &prog);



  static system_call_wrapper_error_t chrome_version(QString &version);
  static const QString &virtual_box_version();
  static const QString &scwe_error_to_str(system_call_wrapper_error_t err);

  static bool set_application_autostart(bool start);
  static bool application_autostart();

  struct container_ip_and_port {
    QString ip;
    QString port;
    bool use_p2p;
  };

  static container_ip_and_port container_ip_from_ifconfig_analog(
      const QString &port, const QString &cont_ip, const QString &rh_ip);
};

#endif  // SYSTEMCALLWRAPPER_H
