#ifndef P2PCONTROLLER_H
#define P2PCONTROLLER_H
#include <set>

#include <QObject>
#include "SystemCallWrapper.h"
#include "HubController.h"
#include "updater/HubComponentsUpdater.h"
#include <QtConcurrent/QtConcurrent>
#include "NotificationObserver.h"
using namespace update_system;
class StatusChecker : public QObject {
  Q_OBJECT
public:
    StatusChecker(QObject *parent = nullptr) : QObject (parent){}
  virtual void startWork() {
    QThread* thread = new QThread;
    this->moveToThread(thread);
    connect(thread, &QThread::started,
            this, &StatusChecker::run_checker);
    connect(this, &StatusChecker::connection_finished,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &StatusChecker::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    thread->start();
  }
private slots:
  virtual void run_checker() = 0;
signals:
  void connection_finished(system_call_wrapper_error_t res);

};
Q_DECLARE_INTERFACE(StatusChecker, "StatusChecker")


class RHStatusChecker : public StatusChecker
{
  Q_OBJECT

public:
  CEnvironment env;
  CHubContainer cont;
  RHStatusChecker(const CEnvironment &_env, const CHubContainer &_cont) : StatusChecker(), env(_env), cont(_cont) {}

private slots:
  void run_checker() {

    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::check_container_state, env.hash(), cont.rh_ip());

    QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,[this, res](){
        emit this->connection_finished(res.result());
    });
  }
};




class SwarmConnector : public StatusChecker
{
  Q_OBJECT
  public:
  CEnvironment env;
  SwarmConnector(const CEnvironment &_env) : StatusChecker(), env(_env) {}

private slots:
  void run_checker() {
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::join_to_p2p_swarm, env.hash(), env.key(),
                          QString("dhcp"), env.base_interface_id());
    QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,[this, res](){
        emit this->connection_finished(res.result());
    });
  }
};



class SwarmLeaver : public StatusChecker
{
  Q_OBJECT
  public:
  QString hash;
  SwarmLeaver(const QString &_hash) : StatusChecker(), hash(_hash) {}

private slots:
  void run_checker() {
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::leave_p2p_swarm, hash);
    QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,[this, res](){
        emit this->connection_finished(res.result());
    });
  }
};

#include "InternalCriticalSection.h"
#include "Locker.h"

class P2PConnector : public QObject {
  Q_OBJECT
public:
 bool env_connected(const QString& env_hash) const {
   SynchroPrimitives::Locker lock(&m_env_critical);
   return connected_envs.find(env_hash) != connected_envs.end();
 }

 bool cont_connected(const QString env_hash, const QString& cont_id) const {
   SynchroPrimitives::Locker lock(&m_cont_critical);
   return connected_conts.find(std::make_pair(env_hash, cont_id)) != connected_conts.end();
 }
 static SynchroPrimitives::CriticalSection m_cont_critical;
 static SynchroPrimitives::CriticalSection m_env_critical;

public slots:
 void update_status();

private:
 std::set< std::pair<QString, QString> > connected_conts; // Connected container. Pair of environment id and container id.
 std::set< QString > connected_envs; // Joined to swarm environment. Id of env is stored
 std::map<int, QString> interface_ids; // Pair of interface id and swarm hash

 int get_unselected_interface_id() {
   for(int id = 0 ; id < 30 ; ++id) { // differents ids for 30 environments
     if (interface_ids.find(id) == interface_ids.end()) {
       return id;
     }
   }
   return -1;
 }

 void join_swarm(const CEnvironment& env);
 void leave_swarm(const QString &hash);
 void check_status(const CEnvironment& env);
 void handshake(const CEnvironment& env, const CHubContainer &cont);
 void check_rh(const CEnvironment& env, const CHubContainer &cont);

};
#include "RestContainers.h"

class P2PController : public QObject
{
Q_OBJECT

public:
  enum P2P_CONNECTION_STATUS{
    CONNECTION_SUCCESS = 0,
    CANT_JOIN_SWARM,
    CANT_CONNECT_CONT,
    P2P_NOT_INSTALLED,
  };

  P2PController();

  static P2PController& Instance() {
    static P2PController instance;
    return instance;
  }

  void init(){/* need to call constructor */}

  P2P_CONNECTION_STATUS is_ready(const CEnvironment&env, const CHubContainer &cont);
  P2P_CONNECTION_STATUS is_swarm_connected(const CEnvironment&env);
  P2P_CONNECTION_STATUS cont_status(const CEnvironment&env, const CHubContainer &cont);
  static QString p2p_connection_status_to_str(P2P_CONNECTION_STATUS status);
  ssh_desktop_launch_error_t is_ready_sdle(const CEnvironment& env, const CHubContainer& cont);
  const std::vector<CP2PInstance> &p2p_instances()const {return m_p2p_instances;}

private slots:
  void p2p_status_updated_sl(std::vector<CP2PInstance> new_p2p_instances,
                             int http_code,
                             int err_code,
                             int network_error);

  void update_p2p_status();

private:
  P2PConnector *connector;
  std::vector<CP2PInstance> m_p2p_instances;

};

/*p2p app status checker*/
class P2PStatus_checker : public QObject
{
Q_OBJECT
public:
  static P2PStatus_checker& Instance() {
    static P2PStatus_checker instance;
    return instance;
  }

  void update_status();

  enum P2P_STATUS {
    P2P_READY = 0,
    P2P_RUNNING,
    P2P_FAIL,
    P2P_LOADING,
    P2P_INSTALLING,
    P2P_UNINSTALLING
   };

   P2PStatus_checker() {
     m_status = P2P_LOADING;
     connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::install_component_started,
             this, &P2PStatus_checker::install_started);
     connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::uninstall_component_started,
             this, &P2PStatus_checker::uninstall_started);
     connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::installing_finished,
             this, &P2PStatus_checker::install_finished);
   }

   P2P_STATUS get_status()  {
     return m_status;
   }

private:
  P2P_STATUS m_status;

signals:
  void p2p_status(P2P_STATUS);

private slots:
  void install_started(const QString &component_id) {
    if (component_id == IUpdaterComponent::P2P) {
      CNotificationObserver::Instance()->Info(tr("P2P installation has started."), DlgNotification::N_NO_ACTION);
      m_status = P2P_INSTALLING;
      emit p2p_status(P2P_INSTALLING);
    }
  }

  void uninstall_started(const QString &component_id) {
    qDebug() << "Uninstall started: "
             << component_id;
    if (component_id == IUpdaterComponent::P2P) {
      CNotificationObserver::Instance()->Info(tr("P2P uninstallation has started."), DlgNotification::N_NO_ACTION);
    }
  }

  void install_finished(const QString &component_id, bool success) {
    UNUSED_ARG(success);
    qDebug() << "install finished "
             << " component: "
             << component_id
             << " success: "
             << success;


    if (component_id == IUpdaterComponent::P2P) {
      m_status = P2P_READY;
      emit p2p_status(P2P_READY);
    }
  }
};

#endif // P2PCONTROLLER_H
