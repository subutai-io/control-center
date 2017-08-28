#ifndef HUBCONTROLLER_H
#define HUBCONTROLLER_H

#include <vector>
#include <QString>
#include <QObject>
#include <QTimer>
#include "Locker.h"
#include "RestContainers.h"
class CEnvironment;
class CHubContainer;
class CHubController;

typedef enum ssh_launch_error {
  SLE_SUCCESS = 0,
  SLE_ENV_NOT_FOUND,
  SLE_CONT_NOT_FOUND,
  SLE_CONT_NOT_READY,
  SLE_JOIN_TO_SWARM_FAILED,
  SLE_SYSTEM_CALL_FAILED,
  SLE_LAST_ERR
} ssh_launch_error_t;

class CHubControllerP2PWorker : public QObject {
  Q_OBJECT
private:
  QString m_env_id;
  QString m_env_hash;
  QString m_env_key;
  QString m_ip;
  QString m_cont_port;
  void *m_additional_data;
public:
  CHubControllerP2PWorker(const QString& env_id,
                          const QString& env_hash,
                          const QString& env_key,
                          const QString& ip,
                          const QString& cont_port,
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
public:
  enum refresh_environments_res_t {
    RER_SUCCESS,
    RER_NO_DIFF,
    RER_EMPTY,
    RER_ERROR
  };

private:
  std::vector<CEnvironment> m_lst_environments_internal;
  std::vector<CEnvironment> m_lst_environments;
  std::vector<CEnvironment> m_lst_healthy_environments;
  std::vector<CMyPeerInfo> m_lst_my_peers;

  QString m_balance;
  QString m_current_user;
  QString m_current_pass;
  QString m_user_id;
  SynchroPrimitives::CriticalSection m_refresh_cs;
  QTimer m_refresh_timer;
  QTimer m_report_timer;

  CHubController();
  ~CHubController();
  CHubController(const CHubController&);
  CHubController& operator=(const CHubController&);

  enum finished_slot_t {
    ssh_to_cont = 0,
    ssh_to_cont_str
  };

  void ssh_to_container_internal(const CEnvironment *env,
                                 const CHubContainer *cont,
                                 void *additional_data,
                                 finished_slot_t slot);

  void refresh_my_peers_internal();
  void refresh_environments_internal();
  void refresh_balance_internal();

private slots:
  void ssh_to_container_finished_slot(int result, void* additional_data);
  void ssh_to_container_finished_str_slot(int result, void* additional_data);

  void settings_changed();

  void report_timer_timeout();
  void refresh_timer_timeout();

  void on_balance_updated_sl(CHubBalance balance,
                             int http_code,
                             int err_code,
                             int network_error);

  void on_environments_updated_sl(std::vector<CEnvironment> lst_environments,
                                  int http_code,
                                  int err_code,
                                  int network_error);

  void on_my_peers_updated_sl(std::vector<CMyPeerInfo> lst_peers,
                              int http_code,
                              int err_code,
                              int network_error);

public:

  void logout();
  void start();
  void force_refresh();

  void ssh_to_container(const CEnvironment *env,
                        const CHubContainer *cont,
                        void *additional_data);

  void ssh_to_container_str(const QString& env_id,
                            const QString& cont_id,
                            void *additional_data);

  static const QString& ssh_launch_err_to_str(int err);

  void set_current_user(const QString& cu) {m_current_user = cu;}

  void set_current_pass(const QString& cp) {m_current_pass = cp;}
  void set_current_user_id(const QString& user_id){ m_user_id = user_id; }

  const std::vector<CEnvironment>& lst_environments() const {return m_lst_environments;}
  const std::vector<CEnvironment>& lst_healthy_environments() const {return m_lst_healthy_environments;}
  const std::vector<CMyPeerInfo>& lst_my_peers() const {return m_lst_my_peers;}

  const QString& balance() const {return m_balance;}

  const QString& current_user() const {return m_current_user;}

  const QString& current_user_id() const {return m_user_id;}
  const QString& current_pass() const {return m_current_pass;}

  void launch_balance_page();
  static CHubController& Instance() {
    static CHubController instance;
    return instance;
  }

signals:
  void ssh_to_container_finished(int result, void* additional_data);
  void ssh_to_container_str_finished(int result, void* additional_data);

  void environments_updated(int);
  void balance_updated();
};

#endif // HUBCONTROLLER_H
