#include "P2PController.h"
#include <QThread>
#include "Locker.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <QFuture>
#include <QFutureWatcher>


void P2PConnector::startInit() {
  update_status_timer = new QTimer(this);
  connect(update_status_timer, &QTimer::timeout,
          this, &P2PConnector::update_status);
  update_status_timer->start(5000); // 20 sec
}


void P2PConnector::join_swarm(const CEnvironment &env) {

  qDebug() << "Trying to join the swarm for env: " << env.name();

  //QFutureWatcher<system_call_wrapper_error_t> *watcher = new QFutureWatcher<system_call_wrapper_error_t> ();

  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::join_to_p2p_swarm, env.hash(), env.key(), QString("dhcp"));
  //watcher->setFuture(res);
  res.waitForFinished();
  //connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [res, env, this]()
  {
    if (res.result() == SCWE_SUCCESS) {
      qInfo() << QString("Joined to swarm [env_name: %1, env_id: %2, swarm_hash: %3]")
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash());
      this->connected_envs.insert(env.hash());
    }
    else {
      qCritical()<< QString("Can't join to swarm [env_name: %1, env_id: %2, swarm_hash: %3] Error message: %4")
                    .arg(env.name())
                    .arg(env.id())
                    .arg(env.hash())
                    .arg(CSystemCallWrapper::scwe_error_to_str(res.result()));
      this->connected_envs.erase(env.hash());
    }
  }
  //);
 //QTimer::singleShot(1, update_status);

}

void P2PConnector::leave_swarm(const QString &hash) {
  qDebug()
      << QString("This swarm is not found in hub environments. Trying to delete. [swarm_hash: %1]")
         .arg(hash);

  //QFutureWatcher<system_call_wrapper_error_t> *watcher = new QFutureWatcher<system_call_wrapper_error_t> ();

  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::leave_p2p_swarm, hash);
  //watcher->setFuture(res);
  res.waitForFinished();
  //connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [res, hash, this]()
  {
    if (res.result() == SCWE_SUCCESS) {
      qInfo() << QString("Left the swarm [swarm_hash: %1]")
                 .arg(hash);
      this->connected_envs.erase(hash);
    }
    else {
      qCritical()<< QString("Can't join to swarm [swarm_hash: %1]. Error message: %2")
                    .arg(hash)
                    .arg(CSystemCallWrapper::scwe_error_to_str(res.result()));
    }
  }
  //);
}

void P2PConnector::handshake(const CEnvironment &env, const CHubContainer &cont) {
  qDebug() << "HANDSHAKING" << env.name() << " " << cont.name();

  //QFutureWatcher<system_call_wrapper_error_t> *watcher = new QFutureWatcher<system_call_wrapper_error_t> ();

  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::send_handshake, cont.rh_ip(), cont.port());
  //watcher->setFuture(res);
  res.waitForFinished();
  //connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [res, env, cont, this]()
  {
    if (res.result() == SCWE_SUCCESS) {
      qInfo() << QString("Successfully handshaked with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]")
                 .arg(cont.name())
                 .arg(cont.id())
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash());
      connected_conts.insert(std::make_pair(env.hash(), cont.id()));
    }
    else {
      qInfo() << QString("Can't handshake with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]. Error message: %6")
                 .arg(cont.name())
                 .arg(cont.id())
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash())
                 .arg(CSystemCallWrapper::scwe_error_to_str(res.result()));
      connected_conts.erase(std::make_pair(env.hash(), cont.id()));
    }
  }
  //);
}

void P2PConnector::check_rh(const CEnvironment &env, const CHubContainer &cont) {
  qDebug() << "checking the rh status" << env.name() << " " << cont.name();


  //QFutureWatcher<system_call_wrapper_error_t> *watcher = new QFutureWatcher<system_call_wrapper_error_t> ();

  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::check_container_state, env.hash(), cont.rh_ip());
  //watcher->setFuture(res);
  res.waitForFinished();

  //connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [res, env, cont, this]()
  {
    if (res.result() == SCWE_SUCCESS) {
      this->handshake(env, cont);
    }
    else {
      qInfo() << QString("The container is not ready.  [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]. Error message: %6")
                 .arg(cont.name())
                 .arg(cont.id())
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash())
                 .arg(CSystemCallWrapper::scwe_error_to_str(res.result()));
    }
  }
  //);

}


void P2PConnector::check_status(const CEnvironment &env) {
  qInfo() <<"Checking the status of environment: " << env.name();
  for (CHubContainer cont : env.containers()) {
    check_rh(env, cont);
  }
}




void P2PConnector::update_status() {
  if (!CCommons::IsApplicationLaunchable(CSettingsManager::Instance().p2p_path())) {
    qInfo() << "Can't launch p2p";
    connected_conts.clear();
    connected_envs.clear();
    return;
  }

  if (!CSystemCallWrapper::p2p_daemon_check()) {
    qInfo() << "P2P daemon is not running";
    connected_conts.clear();
    connected_envs.clear();
    return;
  }

  qInfo() << "Starting to update connection status";


  QFuture<std::vector<QString> > res =
      QtConcurrent::run(CSystemCallWrapper::p2p_show);
  res.waitForFinished();

  std::vector<QString> joined_swarms = res.result();

  std::vector<CEnvironment> hub_environments = CHubController::Instance().lst_environments();
  for (CEnvironment env : hub_environments) {
    if (std::find(joined_swarms.begin(), joined_swarms.end(), env.hash()) == joined_swarms.end()) { // environment not found in joined swarm hashes, we need to try to join it
      connected_envs.erase(env.hash());
      qDebug() << "hello here " << env.name();
      if (env.healthy()) {
        QtConcurrent::run(this, &P2PConnector::join_swarm, env);
      }
    }
    else {
      connected_envs.insert(env.hash());
      QtConcurrent::run(this, &P2PConnector::check_status, env);
    }
  }

  for (QString hash : joined_swarms) {
    auto found_env = std::find_if(hub_environments.begin(), hub_environments.end(), [&hash](const CEnvironment& env) {
      return env.hash() == hash;
    });
    if (found_env == hub_environments.end()) {  // fo
      connected_envs.erase(hash);
      leave_swarm(hash);
    }
  }
}

P2PController::P2PController() {
   qDebug() << "P2P controller is initialized";

   connector = new P2PConnector();
   QThread* thread = new QThread(this);
   connector->moveToThread(thread);

   connect(thread, &QThread::started,
           connector, &P2PConnector::startInit);

   connect(qApp, &QCoreApplication::aboutToQuit,
           thread, &QThread::quit);

   connect(thread, &QThread::finished,
           connector, &P2PConnector::deleteLater);
   connect(thread, &QThread::finished,
           thread, &QThread::deleteLater);

   thread->start();
}



P2PController::P2P_CONNETION_STATUS
P2PController::is_ready(const CEnvironment&env, const CHubContainer &cont) {
  if (!connector->env_connected(env.hash()))
      return CANT_JOIN_SWARM;
  else
  if(!connector->cont_connected(env.hash(), cont.id())) {
    return CANT_CONNECT_CONT;
  }
  else
    return CONNECTION_SUCCESS;
}

QString P2PController::p2p_connection_status_to_str(P2P_CONNETION_STATUS status) {
  static QString str [] = {"Successfully connected",
                           "The connection with environment is not established",
                           "Can't connect to container yet"
                          };
  return str[(size_t) status];
}

ssh_desktop_launch_error_t P2PController::is_ready_sdle(const CEnvironment& env, const CHubContainer& cont) {
  P2P_CONNETION_STATUS ret = is_ready(env, cont);
  static ssh_desktop_launch_error_t res[] = {
    SDLE_SUCCESS,
    SDLE_JOIN_TO_SWARM_FAILED,
    SDLE_CONT_NOT_READY,
  };
  return res[(size_t) ret];
}
