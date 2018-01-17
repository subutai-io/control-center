#include "P2PController.h"
#include <QThread>
#include "Locker.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <QFuture>
#include <QFutureWatcher>






//////////////////////////////////////////////////////////////////////////////////////////////////

void P2PConnector::join_swarm(const CEnvironment &env) {
  qInfo() << "Trying to join the swarm for env: " << env.name();

  SwarmConnector *swarm_connector = new SwarmConnector(env);
  connect(swarm_connector, &SwarmConnector::connection_finished, [this, env](system_call_wrapper_error_t res) {
    if (res == SCWE_SUCCESS) {
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
                    .arg(CSystemCallWrapper::scwe_error_to_str(res));
      this->connected_envs.erase(env.hash());
    }
  });

  swarm_connector->startWork();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void P2PConnector::leave_swarm(const QString &hash) {
  qInfo() << QString("This swarm is not found in hub environments. Trying to delete. [swarm_hash: %1]").arg(hash);

  SwarmLeaver *swarm_leaver = new SwarmLeaver(hash);

  connect(swarm_leaver, &SwarmLeaver::connection_finished, [this, hash](system_call_wrapper_error_t res) {
    this->connected_envs.erase(hash);
    if (res == SCWE_SUCCESS) {
      qInfo() << QString("Left the swarm [swarm_hash: %1]")
                 .arg(hash);
    }
    else {
      qCritical()<< QString("Can't leave the swarm [swarm_hash: %1] Error message: %2")
                    .arg(hash)
                    .arg(CSystemCallWrapper::scwe_error_to_str(res));
    }
  });

  swarm_leaver->startWork();
}


//////////////////////////////////////////////////////////////////////////////////////////////////

void P2PConnector::check_rh(const CEnvironment &env, const CHubContainer &cont) {
  qDebug() << "Checking the rh status" << env.name() << " " << cont.name();

  RHStatusChecker *rh_checker = new RHStatusChecker(env, cont);

  connect(rh_checker, &RHStatusChecker::connection_finished, [this, env, cont](system_call_wrapper_error_t res) {
    if (res == SCWE_SUCCESS) {
      qInfo() << QString("Successfully handshaked with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]")
                 .arg(cont.name())
                 .arg(cont.id())
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash());
      this->connected_conts.insert(std::make_pair(env.hash(), cont.id()));
    }
    else {
      qInfo() << QString("Can't handshake with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]. Error message: %6")
                 .arg(cont.name())
                 .arg(cont.id())
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash())
                 .arg(CSystemCallWrapper::scwe_error_to_str(res));
      this->connected_conts.erase(std::make_pair(env.hash(), cont.id()));
    }
  });
  rh_checker->startWork();
}


void P2PConnector::check_status(const CEnvironment &env) {
  qInfo() <<"Checking the status of environment: " << env.name();

  for (CHubContainer cont : env.containers()) {
    check_rh(env, cont);
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////

void P2PConnector::update_status() {
  QTimer *timer = new QTimer(this); // singleshot timer to call this function, when it finishes to operate
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, this, &P2PConnector::update_status);

  if (!CCommons::IsApplicationLaunchable(CSettingsManager::Instance().p2p_path())
      || !CSystemCallWrapper::p2p_daemon_check()) {
    qCritical() << "P2P is not launchable or p2p daemon is not running.";
    connected_conts.clear();
    connected_envs.clear();
    timer->setInterval(1000);  // wait more, when p2p is not operational
    timer->start();
    return;
  }

  qInfo() << "Starting to update connection status";


  QFuture<std::vector<QString> > res = QtConcurrent::run(CSystemCallWrapper::p2p_show);
  res.waitForFinished();

  std::vector<QString> joined_swarms = res.result();
  std::vector<CEnvironment> hub_environments = CHubController::Instance().lst_environments();

  // joining the swarm
  for (CEnvironment env : hub_environments) {
    if (std::find(joined_swarms.begin(), joined_swarms.end(), env.hash()) == joined_swarms.end()) { // environment not found in joined swarm hashes, we need to try to join it
      connected_envs.erase(env.hash());
      if (env.healthy()) {
        join_swarm(env);
      }
    }
  }

  // checking the status of containers, handshaking
  for (CEnvironment env : hub_environments) {
    if (std::find(joined_swarms.begin(), joined_swarms.end(), env.hash()) != joined_swarms.end()) { // environment not found in joined swarm hashes, we need to try to join it
      connected_envs.insert(env.hash());
      check_status(env);
    }
  }

  // checking deleted environments
  for (QString hash : joined_swarms) {
    auto found_env = std::find_if(hub_environments.begin(), hub_environments.end(), [&hash](const CEnvironment& env) {
      return env.hash() == hash;
    });
    if (found_env == hub_environments.end()) {
      connected_envs.erase(hash);
      leave_swarm(hash);
    }
  }

  timer->setInterval(1000);
  timer->start();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

P2PController::P2PController() {
   qDebug() << "P2P controller is initialized";

   connector = new P2PConnector();
   QThread* thread = new QThread(this);
   connector->moveToThread(thread);

   connect(thread, &QThread::started,
           connector, &P2PConnector::update_status);

   connect(qApp, &QCoreApplication::aboutToQuit,
           thread, &QThread::quit);

   connect(thread, &QThread::finished,
           connector, &P2PConnector::deleteLater);
   connect(thread, &QThread::finished,
           thread, &QThread::deleteLater);

   QTimer::singleShot(5000, [thread](){ // Chance that the connection with hub is established after 5 sec is high
     thread->start();
   });
}



P2PController::P2P_CONNETION_STATUS
P2PController::is_ready(const CEnvironment&env, const CHubContainer &cont) {
  if (!connector->env_connected(env.hash()))
      return CANT_JOIN_SWARM;
  else
  if(!connector->cont_connected(env.hash(), cont.id()))
    return CANT_CONNECT_CONT;
  else
    return CONNECTION_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

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
