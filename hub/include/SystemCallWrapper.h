#ifndef SYSTEMCALLWRAPPER_H
#define SYSTEMCALLWRAPPER_H

#include <string>
#include <vector>
#include <QString>
#include <QObject>

enum system_call_wrapper_error_t {
  /*common errors*/
  SCWE_SUCCESS = 0,
  SCWE_SHELL_ERROR,
  SCWE_PIPE,
  SCWE_SET_HANDLE_INFO,
  SCWE_CREATE_PROCESS,

  /*p2p errors*/
  SCWE_CANT_JOIN_SWARM,

  /*ssh errors*/
  SCWE_SSH_LAUNCH_FAILED,
};
////////////////////////////////////////////////////////////////////////////

class CSystemCallThreadWrapper : public QObject {
  Q_OBJECT
private:
  system_call_wrapper_error_t m_result;
  int m_exit_code;
  std::string m_command;
  std::vector<std::string> m_lst_output;

public:
  CSystemCallThreadWrapper(QObject* parent = 0) : QObject(parent), m_result(SCWE_SUCCESS), m_exit_code(0), m_command("") {}
  CSystemCallThreadWrapper(const char *command, QObject *parent = 0) :
    QObject(parent),
    m_result(SCWE_SUCCESS),
    m_exit_code(0),
    m_command(command){}


  system_call_wrapper_error_t result() const { return m_result;}
  int exit_code() const {return m_exit_code;}
  std::vector<std::string> lst_output() const {return m_lst_output;}

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
                                                int &exit_code);

  static system_call_wrapper_error_t ssystem(const char *command,
                                             std::vector<std::string> &lst_output,
                                             int& exit_code);

public:

  static bool is_in_swarm(const char* hash);

  static system_call_wrapper_error_t join_to_p2p_swarm(const char* hash,
                                                       const char* key,
                                                       const char* ip);

  static system_call_wrapper_error_t run_ssh_in_terminal(const char *user,
                                                         const char *ip,
                                                         const char *port);

  static system_call_wrapper_error_t run_ss_updater(const char* host,
                                                    const char* port,
                                                    const char* user,
                                                    const char* pass,
                                                    const char* cmd, int &exit_code);

  static system_call_wrapper_error_t fork_process(const QString program,
                                                  const QStringList argv,
                                                  const QString& folder);
  static system_call_wrapper_error_t open_url(QString s_url);
};

#endif // SYSTEMCALLWRAPPER_H

