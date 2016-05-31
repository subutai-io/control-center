#ifndef HUBCONTROLLER_H
#define HUBCONTROLLER_H

#include <vector>
#include <QString>
#include <QObject>
#include "Locker.h"

class CSSEnvironment;
class CRHInfo;
class CHubContainer;

typedef enum ssh_launch_error {
  SLE_SUCCESS = 0,
  SLE_ENV_NOT_FOUND,
  SLE_CONT_NOT_FOUND,
  SLE_JOIN_TO_SWARM_FAILED,
  SLE_SYSTEM_CALL_FAILED,
  SLE_LAST_ERR
} ssh_launch_error_t;

class CHubControllerP2PWorker : public QObject {
  Q_OBJECT
private:
  std::string m_env_hash;
  std::string m_env_key;
  std::string m_ip;
  std::string m_cont_port;
  void *m_additional_data;
public:
  CHubControllerP2PWorker(const std::string& env_hash,
                          const std::string& env_key,
                          const std::string& ip,
                          const std::string& cont_port,
                          void* additional_data);
  ~CHubControllerP2PWorker();
private slots:
public slots:
  void join_to_p2p_swarm_begin();
  void ssh_to_container_begin(int join_result);
signals:
  void join_to_p2p_swarm_finished(int result);
  void ssh_to_container_finished(int result, void *additional_data);

};
////////////////////////////////////////////////////////////////////////////

class CHubController : public QObject {
  Q_OBJECT
private:
  std::vector<CSSEnvironment> m_lst_environments;
  std::vector<CRHInfo> m_lst_resource_hosts;
  QString m_balance;
  QString m_current_user;
  QString m_current_pass;

  SynchroPrimitives::CriticalSection m_refresh_cs;

  CHubController();
  ~CHubController();
  CHubController(const CHubController&);
  CHubController& operator=(const CHubController&);

  enum finished_slot_t {
    ssh_to_cont = 0,
    ssh_to_cont_str
  };

  void ssh_to_container_internal(const CSSEnvironment *env,
                                 const CHubContainer *cont,
                                 void *additional_data,
                                 finished_slot_t slot);

private slots:
  void ssh_to_container_finished_slot(int result, void* additional_data);
  void ssh_to_container_finished_str_slot(int result, void* additional_data);

public:
  int refresh_balance();
  int refresh_environments();
  void refresh_containers(); //to make ssh work

  void ssh_to_container(const CSSEnvironment *env,
                        const CHubContainer *cont,
                        void *additional_data);

  void ssh_to_container_str(const QString& env_id,
                            const QString& cont_id,
                            void *additional_data);

  static const QString& ssh_launch_err_to_str(int err);

  void set_current_user(const QString& cu) {m_current_user = cu;}
  void set_current_pass(const QString& cp) {m_current_pass = cp;}

  const std::vector<CSSEnvironment>& lst_environments() const { return m_lst_environments; }
  const std::vector<CRHInfo>& lst_resource_hosts() const { return m_lst_resource_hosts; }
  const QString& balance() const {return m_balance;}

  const QString& current_user() const {return m_current_user;}
  const QString& current_pass() const {return m_current_pass;}

  static CHubController& Instance() {
    static CHubController instance;
    return instance;
  }

signals:
  void ssh_to_container_finished(int result, void* additional_data);
  void ssh_to_container_str_finished(int result, void* additional_data);
};

#endif // HUBCONTROLLER_H
