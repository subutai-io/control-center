#ifndef SYSTEMCALLWRAPPER_H
#define SYSTEMCALLWRAPPER_H

#include <string>
#include <vector>
#include <QString>
#include <QObject>
#include <stdint.h>

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
  SCWE_CANT_JOIN_SWARM,
  SCWE_CONTAINER_IS_NOT_READY,

  /*ssh errors*/
  SCWE_SSH_LAUNCH_FAILED,
  SCWE_CANT_GET_RH_IP,
  SCWE_CANT_GENERATE_SSH_KEY,

  /*other errors*/
  SCWE_TIMEOUT,
  SCWE_WHICH_CALL_FAILED,
  SCWE_PROCESS_CRASHED
};
////////////////////////////////////////////////////////////////////////////

enum restart_service_error_t {
  RSE_SUCCESS,
  RSE_MANUAL
};
////////////////////////////////////////////////////////////////////////////

class CSystemCallThreadWrapper : public QObject {
  Q_OBJECT
private:
  system_call_wrapper_error_t m_result;
  int m_exit_code;
  QString m_command;
  QStringList m_args;
  QStringList m_lst_output;
  bool m_read_output;

public:
  CSystemCallThreadWrapper(QObject* parent = 0) : QObject(parent), m_result(SCWE_SUCCESS), m_exit_code(0),
    m_command(""), m_read_output(true) {}
  CSystemCallThreadWrapper(const QString& command, const QStringList &args, bool arg_read_output, QObject *parent = 0) :
    QObject(parent),
    m_result(SCWE_SUCCESS),
    m_exit_code(0),
    m_command(command),
    m_args(args),
    m_read_output(arg_read_output){}


  system_call_wrapper_error_t result() const { return m_result;}
  int exit_code() const {return m_exit_code;}
  const QStringList& lst_output() const {return m_lst_output;}
  bool read_output() const {return m_read_output;}

public slots:
  void do_system_call();

signals:
  void finished();
};
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This class contains methods and functions for receiving output of system calls
 * like "ls -la", "p2p version" etc.
 */
class CSystemCallWrapper {
  friend class CSystemCallThreadWrapper;
private:

  static system_call_wrapper_error_t run_libssh2_command(const char *host,
                                                         uint16_t port,
                                                         const char *user,
                                                         const char *pass,
                                                         const char *cmd,
                                                         int& exit_code,
                                                         std::vector<std::string> &lst_output);
public:

  static system_call_wrapper_error_t ssystem_th(const QString &cmd,
                                                 const QStringList &args,
                                                 QStringList &lst_out,
                                                 int &exit_code,
                                                 bool read_output,
                                                 unsigned long timeout_msec = ULONG_MAX);

  static system_call_wrapper_error_t ssystem(const QString& cmd,
                                              const QStringList& args,
                                              QStringList &lst_output,
                                              int &exit_code,
                                              bool read_output);

  static bool is_in_swarm(const QString &hash);

  static system_call_wrapper_error_t join_to_p2p_swarm(const QString &hash,
                                                       const QString &key,
                                                       const QString &ip);

  static system_call_wrapper_error_t leave_p2p_swarm(const QString &hash);
  static system_call_wrapper_error_t restart_p2p_service(int *res_code);

  static system_call_wrapper_error_t check_container_state(const QString &hash,
                                                           const QString &ip);

  static system_call_wrapper_error_t run_ssh_in_terminal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &key);

  static system_call_wrapper_error_t generate_ssh_key(const QString &comment,
                                                      const QString &file_path);

  static system_call_wrapper_error_t is_rh_update_available(bool& available);
  static system_call_wrapper_error_t is_rh_management_update_available(bool& available);

  static system_call_wrapper_error_t run_rh_updater(const char* host,
                                                    uint16_t port,
                                                    const char* user,
                                                    const char* pass,
                                                    int &exit_code);

  static system_call_wrapper_error_t run_rh_management_updater(const char* host,
                                                               uint16_t port,
                                                               const char* user,
                                                               const char* pass,
                                                               int &exit_code);

  static system_call_wrapper_error_t get_rh_ip_via_libssh2(const char* host,
                                                           uint16_t port,
                                                           const char* user,
                                                           const char* pass,
                                                           int &exit_code,
                                                           std::string& ip);

  static QString rh_version();
  static QString rhm_version();

  static system_call_wrapper_error_t p2p_version(QString &version);
  static system_call_wrapper_error_t p2p_status(QString &status);

  static system_call_wrapper_error_t which(const QString &prog,
                                           QString &path);

  static system_call_wrapper_error_t chrome_version(QString& version);
  static QString virtual_box_version();
  static const QString& scwe_error_to_str(system_call_wrapper_error_t err);
};

#endif // SYSTEMCALLWRAPPER_H

