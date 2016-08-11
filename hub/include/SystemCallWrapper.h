#ifndef SYSTEMCALLWRAPPER_H
#define SYSTEMCALLWRAPPER_H

#include <string>
#include <vector>
#include <QString>
#include <QObject>

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
  SCWE_WHICH_CALL_FAILED
};
////////////////////////////////////////////////////////////////////////////

class CSystemCallThreadWrapper : public QObject {
  Q_OBJECT
private:
  system_call_wrapper_error_t m_result;
  int m_exit_code;
  std::string m_command;
  std::vector<std::string> m_lst_output;
  bool m_read_output;

public:
  CSystemCallThreadWrapper(QObject* parent = 0) : QObject(parent), m_result(SCWE_SUCCESS), m_exit_code(0),
    m_command(""), m_read_output(true) {}
  CSystemCallThreadWrapper(const char *command, bool arg_read_output, QObject *parent = 0) :
    QObject(parent),
    m_result(SCWE_SUCCESS),
    m_exit_code(0),
    m_command(command),
    m_read_output(arg_read_output){}


  system_call_wrapper_error_t result() const { return m_result;}
  int exit_code() const {return m_exit_code;}
  std::vector<std::string> lst_output() const {return m_lst_output;}
  bool read_output() const {return m_read_output;}

public slots:
  void do_system_call();

signals:
  void finished();
};
////////////////////////////////////////////////////////////////////////////

class CSystemCallWrapper {
  friend class CSystemCallThreadWrapper;
private:  

  static system_call_wrapper_error_t ssystem_th(const char *command,
                                                std::vector<std::string> &lst_output,
                                                int &exit_code, bool read_output, unsigned long time_msec = ULONG_MAX);

  static system_call_wrapper_error_t ssystem(const char *command,
                                             std::vector<std::string> &lst_output,
                                             int& exit_code, bool read_output = true);

  static system_call_wrapper_error_t run_libssh2_command(const char *host,
                                                         const char *port,
                                                         const char *user,
                                                         const char *pass,
                                                         const char *cmd,
                                                         int& exit_code,
                                                         std::vector<std::string> &lst_output);
public:


  static bool is_in_swarm(const char* hash);

  static system_call_wrapper_error_t join_to_p2p_swarm(const char* hash,
                                                       const char* key,
                                                       const char* ip);

  static system_call_wrapper_error_t leave_p2p_swarm(const char* hash);

  static system_call_wrapper_error_t check_container_state(const char* hash,
                                                           const char* ip);

  static system_call_wrapper_error_t run_ssh_in_terminal(const char *user,
                                                         const char *ip,
                                                         const char *port,
                                                         const char *key);

  static system_call_wrapper_error_t generate_ssh_key(const char *comment,
                                                      const char *file_path);

  static system_call_wrapper_error_t is_rh_update_available(bool& available);

  static system_call_wrapper_error_t run_ss_updater(const char* host,
                                                    const char* port,
                                                    const char* user,
                                                    const char* pass,
                                                    int &exit_code);

  static system_call_wrapper_error_t get_rh_ip_via_libssh2(const char* host,
                                                           const char* port,
                                                           const char* user,
                                                           const char* pass,
                                                           int &exit_code,
                                                           std::string& ip);

  static system_call_wrapper_error_t fork_process(const QString program,
                                                  const QStringList argv,
                                                  const QString& folder);
  static system_call_wrapper_error_t open_url(QString s_url);

  static QString rh_version();

  static system_call_wrapper_error_t p2p_version(std::string& version);
  static system_call_wrapper_error_t p2p_status(std::string& status);

  //where on windows :)
  static system_call_wrapper_error_t which(const std::string& prog,
                                           std::string& path);

  static system_call_wrapper_error_t chrome_version(std::string& version);
  static QString virtual_box_version();
  static const QString& scwe_error_to_str(system_call_wrapper_error_t err);

};

#endif // SYSTEMCALLWRAPPER_H

