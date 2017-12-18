#include "P2PController.h"
#include <QThread>
#include "Locker.h"
#include <QDebug>




/////////////////////////////////////////////////////////////////////////

SwarmConnector::SwarmConnector(const CEnvironment &_env) : env(_env) {
  qDebug() << QString("SwarmConnector with [env_name: %1, env_id: %2, swarm_hash: %3] and key: %4")
                .arg(env.name())
                .arg(env.id())
                .arg(env.hash())
                .arg(env.key());
  QTimer *timer = new QTimer();
  timer->setInterval(50 * 1000); // 50 sec
  connect(timer, &QTimer::timeout,
          this, &SwarmConnector::join_to_swarm_begin);
  timer->start();
}

void SwarmConnector::join_to_swarm_begin() {
  system_call_wrapper_error_t res = CSystemCallWrapper::join_to_p2p_swarm(env.hash(), env.key(), "dhcp");
  if (res == SCWE_SUCCESS) {
    emit successfully_joined_swarm(env);
    qInfo() << QString("Joined to swarm [env_name: %1, env_id: %2, swarm_hash: %3]")
               .arg(env.name())
               .arg(env.id())
               .arg(env.hash());
  }
  else {
    qCritical()<< QString("Can't join to swarm [env_name: %1, env_id: %2, swarm_hash: %3] Error message: %4")
                  .arg(env.name())
                  .arg(env.id())
                  .arg(env.hash())
                  .arg(CSystemCallWrapper::scwe_error_to_str(res).toStdString().c_str());
  }
}

/////////////////////////////////////////////////////////////////////////

HandshakeSender::HandshakeSender(const CEnvironment &_env) : env(_env) {
  QTimer *timer = new QTimer;
  timer->setInterval(30 * 1000); // 30 sec
  connect(timer, &QTimer::timeout, this, &HandshakeSender::handshake_begin);
  timer->start();
}

void HandshakeSender::try_to_handshake(const CHubContainer &cont) {
  qDebug() << QString("Trying to handshake with container: [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]")
              .arg(cont.name())
              .arg(cont.id())
              .arg(env.name())
              .arg(env.id())
              .arg(env.hash());

  system_call_wrapper_error_t err =
      CSystemCallWrapper::check_container_state(env.hash(), cont.rh_ip());

  if (err == SCWE_SUCCESS)
    err = CSystemCallWrapper::send_handshake(cont.rh_ip(), cont.port());

  if (err == SCWE_SUCCESS) {
    qInfo() << QString("Successfully handshaked with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]")
               .arg(cont.name())
               .arg(cont.id())
               .arg(env.name())
               .arg(env.id())
               .arg(env.hash());
    emit handshake_success(env.id(), cont.id());
  }
  else {
    qInfo() << QString("Can't handshake with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]. Error message: %3")
               .arg(cont.name())
               .arg(cont.id())
               .arg(env.name())
               .arg(env.id())
               .arg(env.hash())
               .arg(CSystemCallWrapper::scwe_error_to_str(err));
    emit handshake_failure(env.id(), cont.id());
  }
}

void HandshakeSender::handshake_begin(){
  for (CHubContainer cont : env.containers()) {
    try_to_handshake(cont);
  }
}



/////////////////////////////////////////////////////////////////////////


P2PController::P2PController() {
  qDebug() << "P2PController is initialized";
  m_join_to_swarm_timer = new QTimer(this);
  connect(m_join_to_swarm_timer, &QTimer::timeout, this, &P2PController::update_join_swarm_status);
  m_join_to_swarm_timer->start(1000 * 60 * 5); // 5 minutes
  QTimer::singleShot(15000, this, &P2PController::update_join_swarm_status); // 15 sec
}

P2PController::~P2PController() {
  qDebug() << "P2PController destructor";
  if (!envs_joined_swarm_hash.empty() && CSystemCallWrapper::p2p_daemon_check()) {
    for (auto env_hash : envs_joined_swarm_hash) {
      system_call_wrapper_error_t res = CSystemCallWrapper::leave_p2p_swarm(env_hash);
      if (res == SCWE_SUCCESS) {
        qInfo() << QString("Left the swarm [swarm_hash: %1]")
                    .arg(env_hash);
      }
      else {
        qCritical() << QString("Can't leave the swarm [swarm_hash: %1]. Error message %2")
                       .arg(env_hash)
                       .arg(CSystemCallWrapper::scwe_error_to_str(res).toStdString().c_str());
      }
    }
  }
  envs_joined_swarm_hash.clear();
}

/////////////////////////////////////////////////////////////////////////
/* Handshake Senders */

void P2PController::handshaked(QString env_id, QString cont_id) {
  successfull_handshakes.insert(std::make_pair(env_id, cont_id));
}

bool P2PController::handshake_success(QString env_id, QString cont_id) {
  return successfull_handshakes.find(std::make_pair(env_id, cont_id)) != successfull_handshakes.end();
}


void P2PController::handshake_failed(QString env_id, QString cont_id) {
  successfull_handshakes.erase(std::make_pair(env_id, cont_id));
}

void P2PController::handshake_with_env(const CEnvironment &env) {
  qDebug() << QString("Create instance for handshaking for [env_name: %1, env_id: %2, swarm_hash: %3]")
           .arg(env.name())
           .arg(env.id())
           .arg(env.hash());

  HandshakeSender* handshaker = new HandshakeSender(env);
  QThread* thread = new QThread();
  handshaker->moveToThread(thread);
  connect(thread, &QThread::started, handshaker, &HandshakeSender::handshake_begin);
  connect(handshaker, &HandshakeSender::handshake_success, this, &P2PController::handshaked);
  connect(handshaker, &HandshakeSender::handshake_failure, this, &P2PController::handshake_failed);
  connect(thread, &QThread::finished, handshaker, &SwarmConnector::deleteLater);
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

/////////////////////////////////////////////////////////////////////////

bool P2PController::has_connector(QString env_id) {
  return envs_with_connectors.find(env_id) != envs_with_connectors.end();
}

std::vector<CEnvironment> P2PController::get_envs_without_connectors(){
  std::vector<CEnvironment> current_envs = CHubController::Instance().lst_healthy_environments();
  std::vector<CEnvironment> ret_envs;
  for (CEnvironment &env : current_envs) {
    if (!has_connector(env.id())) {
      ret_envs.push_back(env);
    }
  }
  return ret_envs;
}



/////////////////////////////////////////////////////////////////////////////////////////
/* Swarm Joiner triggers */

bool P2PController::join_swarm_success(QString swarm_hash) {
  return envs_joined_swarm_hash.find(swarm_hash) != envs_joined_swarm_hash.end();
}

void P2PController::joined_swarm(const CEnvironment &env) {
  envs_joined_swarm_hash.insert(env.hash());
}

void P2PController::join_swarm(const CEnvironment &env) {
  qDebug() << QString("Create instance for joining to swarm [env_name: %1, env_id: %2, swarm_hash: %3]")
              .arg(env.name())
              .arg(env.id())
              .arg(env.hash());

  SwarmConnector* connector = new SwarmConnector(env);
  QThread* thread = new QThread();
  connector->moveToThread(thread);
  connect(thread, &QThread::started, connector, &SwarmConnector::join_to_swarm_begin);
  connect(connector, &SwarmConnector::successfully_joined_swarm, this, &P2PController::joined_swarm);
  connect(connector, &SwarmConnector::successfully_joined_swarm, this, &P2PController::handshake_with_env);
  connect(connector, &SwarmConnector::successfully_joined_swarm, thread, &QThread::quit);
  connect(thread, &QThread::finished, connector, &SwarmConnector::deleteLater);
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();

  envs_with_connectors.insert(env.id());
}

void P2PController::update_join_swarm_status(){
  if (!CSystemCallWrapper::p2p_daemon_check()) {
    return;
  }

  std::vector<CEnvironment> current_envs = get_envs_without_connectors();

  qDebug() << "Healthy envs not joined to swarm: " << current_envs.size();
  qDebug() << "All healthy envs count: "<< (CHubController::Instance().lst_healthy_environments()).size();
  qDebug() << "All envs count: "<< (CHubController::Instance().lst_environments()).size();

  for (CEnvironment env : current_envs) {
    join_swarm(env);
  }

}

/////////////////////////////////////////////////////////////////////////
/* P2PContrller HELPERS */

void P2PController::p2p_restart() {
  envs_joined_swarm_hash.clear();
  successfull_handshakes.clear();
  envs_with_connectors.clear();
}


int P2PController::get_container_status(const CEnvironment *env, const CHubContainer *cont) {
  if (env == NULL)
    return (int) SDLE_ENV_NOT_FOUND;

  if (cont == NULL)
    return (int) SDLE_CONT_NOT_FOUND;

  if (!P2PController::Instance().join_swarm_success(env->hash()))
    return (int) SDLE_JOIN_TO_SWARM_FAILED;

  if (!P2PController::Instance().handshake_success(env->id(), cont->id()))
    return (int) SDLE_CONT_NOT_READY;

  return (int) SDLE_SUCCESS;
}
