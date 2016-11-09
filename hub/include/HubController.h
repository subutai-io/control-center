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

/*!
 * \brief This class is used for managing HUB related thins.
 * It updates environments and containers lists, user's balance.
 * Also it is used for doint ssh->container.
 */
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
  /*!
   * \brief Force refresh balance
   */
  int refresh_balance();

  enum refresh_environments_res_t {
    RER_SUCCESS,
    RER_NO_DIFF,
    RER_EMPTY,
    RER_ERROR
  };

  /*!
   * \brief Force refresh environments list
   */
  refresh_environments_res_t refresh_environments();

  /*!
   * \brief Force refresh containers list
   */
  void refresh_containers(); //to make ssh work

  /*!
   * \brief SSH->container
   * \param env - pointer to environment
   * \param cont - pointer to container
   * \param additional_data - could be everything you want. here we use it for storing information about menu item
   */
  void ssh_to_container(const CSSEnvironment *env,
                        const CHubContainer *cont,
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
  const std::vector<CSSEnvironment>& lst_environments() const { return m_lst_environments; }

  /*!
   * \brief List of resource hosts. This list is used for searching containers IP.
   */
  const std::vector<CRHInfo>& lst_resource_hosts() const { return m_lst_resource_hosts; }

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
};

#endif // HUBCONTROLLER_H
