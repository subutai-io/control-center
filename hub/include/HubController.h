#ifndef HUBCONTROLLER_H
#define HUBCONTROLLER_H

#include <vector>
#include <QString>
#include <QObject>
#include <QTimer>
#include "Locker.h"

class CEnvironment;
class CRHInfo;
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

/*!
 * \brief This class is used for working with p2p. At first we need to join to p2p and then to try ssh->container.
 * We do it in separate thread so we have signals here.
 */
class CHubControllerP2PWorker : public QObject {
  Q_OBJECT
private:
  QString m_env_hash;
  QString m_env_key;
  QString m_ip;
  QString m_cont_port;
  void *m_additional_data;
public:
  CHubControllerP2PWorker(const QString& env_hash,
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


class CHubContainerEx {
private:
  QString m_name;
  QString m_ip;
  QString m_id;
  QString m_port;
  QString m_rh_ip;

public:

  CHubContainerEx(const CHubContainer& cont,
                  const std::vector<CRHInfo>& lst_resource_hosts);
  ~CHubContainerEx(){}

  const QString& name() const {return m_name;}
  const QString& ip() const {return m_ip;}
  const QString& id() const {return m_id;}
  const QString& port() const {return m_port;}
  const QString& rh_ip() const {return m_rh_ip;}
};
////////////////////////////////////////////////////////////////////////////

class CEnvironmentEx {
private:
  QString m_name;
  QString m_hash;
  QString m_aes_key;
  QString m_ttl;
  QString m_id;
  QString m_status;
  QString m_status_descr;
  std::vector<CHubContainerEx> m_lst_containers;

public:

  CEnvironmentEx(const CEnvironment& env,
                 const std::vector<CRHInfo>& lst_resource_hosts);
  ~CEnvironmentEx() {}

  const QString& name() const {return m_name;}
  const QString& hash() const {return m_hash;}
  const QString& key() const {return m_aes_key;}
  const QString& ttl() const {return m_ttl;}
  const QString& id() const {return m_id;}
  const QString& status() const {return m_status;}
  const QString& status_description() const {return m_status_descr;}
  const std::vector<CHubContainerEx>& containers() const {return m_lst_containers;}
  bool healthy() const {return m_status == "HEALTHY";}
};
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This class is used for managing HUB related thins.
 * It updates environments and containers lists, user's balance.
 * Also it is used for doint ssh->container.
 */
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
  std::vector<CRHInfo> m_lst_resource_hosts;

  std::vector<CEnvironmentEx> m_lst_environments;

  QString m_balance;
  QString m_current_user;
  QString m_current_pass;
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

  void ssh_to_container_internal(const CEnvironmentEx *env,
                                 const CHubContainerEx *cont,
                                 void *additional_data,
                                 finished_slot_t slot);

  void refresh_containers_internal();
  refresh_environments_res_t refresh_environments_internal();
  int refresh_balance();

private slots:
  void ssh_to_container_finished_slot(int result, void* additional_data);
  void ssh_to_container_finished_str_slot(int result, void* additional_data);

  void refresh_timer_timeout();
  void settings_changed();
  void report_timer_timeout();

public:

  void suspend() {m_refresh_timer.stop();}
  void start() {m_refresh_timer.start();}
  void force_refresh();

  /*!
   * \brief SSH->container
   * \param env - pointer to environment
   * \param cont - pointer to container
   * \param additional_data - could be everything you want. here we use it for storing information about menu item
   */
  void ssh_to_container(const CEnvironmentEx *env,
                        const CHubContainerEx *cont,
                        void *additional_data);

  /*!
   * \brief This method is used for ssh->container when we don't have pointer to environment and container, but have ids.
   * \param env_id - environment id string
   * \param cont_id - container id string
   * \param additional_data - could be everything you want. here we use it for storing information about menu item
   */
  void ssh_to_container_str(const QString& env_id,
                            const QString& cont_id,
                            void *additional_data);

  /*!
   * \brief ssh->container error to string
   */
  static const QString& ssh_launch_err_to_str(int err);

  /*!
   * \brief Current user here is login. This field is used for re-login to hub.
   */
  void set_current_user(const QString& cu) {m_current_user = cu;}

  /*!
   * \brief Current password. This field is used for re-login to hub.
   */
  void set_current_pass(const QString& cp) {m_current_pass = cp;}

  /*!
   * \brief List of environments
   */
  const std::vector<CEnvironmentEx>& lst_environments() const {return m_lst_environments;}

  /*!
   * \brief Current balance represented by string
   */
  const QString& balance() const {return m_balance;}

  /*!
   * \brief Current user's login
   */
  const QString& current_user() const {return m_current_user;}

  /*!
   * \brief Current user's password
   */
  const QString& current_pass() const {return m_current_pass;}

  /*!
   * \brief Instance of this singleton class
   */
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
