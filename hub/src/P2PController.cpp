#include "P2PController.h"
#include <QThread>
#include "Locker.h"
#include <QDebug>



/////////////////////////////////////////////////////////////////////////

SwarmConnector::~SwarmConnector() {
  qDebug() << QString("SwarmConnector desctructor");
}

/////////////////////////////////////////////////////////////////////////

SwarmConnector::SwarmConnector(QString swarm_hash, QString swarm_key) : swarm_hash(swarm_hash) , swarm_key(swarm_key) {
  qDebug() << QString("SwarmConnector with hash: %1 and key: %2").arg(swarm_hash).arg(swarm_key);
}

///////////////////////////////////////////////////////////////

void SwarmConnector::join_to_swarm_begin() {
  system_call_wrapper_error_t res = CSystemCallWrapper::join_to_p2p_swarm(swarm_hash, swarm_key, "dhcp");
  if (res == SCWE_SUCCESS) {
    emit successfully_joined_swarm(swarm_hash);
    qInfo("Joined to swarm %s.",
              swarm_hash.toStdString().c_str());
  }
  else {
    qCritical("Can't join to swarm %s. Err %s",
              swarm_hash.toStdString().c_str(),
              CSystemCallWrapper::scwe_error_to_str(res).toStdString().c_str());
  }
  emit join_to_swarm_finished();
}

/////////////////////////////////////////////////////////////////////////

void HandshakeSender::try_to_handshake(const CEnvironment &env, const CHubContainer &cont) {
  qDebug() << QString("Trying to handshake with cont: %1 and env: %2").arg(cont.name()).arg(env.name());

  system_call_wrapper_error_t err =
      CSystemCallWrapper::check_container_state(env.hash(), cont.rh_ip());

  if (err == SCWE_SUCCESS)
    err = CSystemCallWrapper::send_handshake(cont.rh_ip(), cont.port());

  if (err == SCWE_SUCCESS) {
    qInfo("Successfully handshaked with container %s from environment %s",
          cont.name().toStdString().c_str(),
          env.name().toStdString().c_str());
    emit handshake_success(env.id(), cont.id());

  }
  else {
    qInfo("Can't handshake with container %s from environment %s. Err: %s",
          cont.name().toStdString().c_str(),
          env.name().toStdString().c_str(),
          CSystemCallWrapper::scwe_error_to_str(err).toStdString().c_str());
    emit handshake_failure(env.id(), cont.id());
  }
}

void HandshakeSender::send_handshakes(){
  for (CEnvironment &env : m_envs) {
    for (CHubContainer cont : env.containers()) {
      try_to_handshake(env, cont);
    }
  }
  emit sent_handshakes_succsessfully();
}


HandshakeSender::HandshakeSender(const std::vector<CEnvironment> envs) : m_envs(envs) {
  qDebug() << "Handshake Sender with envs";
}

HandshakeSender::~HandshakeSender() {}
/////////////////////////////////////////////////////////////////////////

P2PController::~P2PController() {
  qDebug("P2PController desctructor");
  if (!envs_joined_swarm_hash.empty() && CSystemCallWrapper::p2p_daemon_check()) {
    for (auto env_hash : envs_joined_swarm_hash) {
      system_call_wrapper_error_t res = CSystemCallWrapper::leave_p2p_swarm(env_hash);
      if (res == SCWE_SUCCESS) {
        qInfo("Left the swarm %s.",
                  env_hash.toStdString().c_str());
      }
      else {
        qCritical("Can't leave the swarm %s. Err %s",
                  env_hash.toStdString().c_str(),
                  CSystemCallWrapper::scwe_error_to_str(res).toStdString().c_str());
      }
    }
  }
  envs_joined_swarm_hash.clear();
}

P2PController::P2PController() {
  qDebug("P2PController is initialized");

  m_join_to_swarm_timer = new QTimer(this);
  m_handshake_timer = new QTimer(this);

  connect(m_join_to_swarm_timer, &QTimer::timeout,
          this, &P2PController::update_join_swarm_status);
  connect(m_handshake_timer, &QTimer::timeout,
          this, &P2PController::update_handshake_status);

  m_join_to_swarm_timer->start(1000 * 50); // 50 sec
  m_handshake_timer->start(1000 * 30); // 30 sec
  QTimer::singleShot(10000, this, SLOT(update_join_swarm_status())); // this functions call after 10 sec. single time
}

/////////////////////////////////////////////////////////////////////////

void P2PController::joined_swarm(QString hash) {
  envs_joined_swarm_hash.insert(hash);
}

void P2PController::join_swarm(const CEnvironment &env) {
  qDebug() << "Create instance for joining to swarm for env name:" << env.name() << "with hash: " << env.hash();

  SwarmConnector* connector = new SwarmConnector(env.hash(), env.key());
  QThread* thread = new QThread();
  connector->moveToThread(thread);

  connect(thread, &QThread::started, connector, &SwarmConnector::join_to_swarm_begin);
  connect(connector, &SwarmConnector::successfully_joined_swarm,
          this, &P2PController::joined_swarm);
  connect(connector, &SwarmConnector::join_to_swarm_finished, thread, &QThread::quit);
  connect(thread, &QThread::finished, connector, &SwarmConnector::deleteLater);
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

/////////////////////////////////////////////////////////////////////////

void P2PController::handshaked(QString env_id, QString cont_id) {
  successfull_handshakes.insert(std::make_pair(env_id, cont_id));
}

void P2PController::handshake_failed(QString env_id, QString cont_id) {
  successfull_handshakes.erase(std::make_pair(env_id, cont_id));
}

/////////////////////////////////////////////////////////////////////////

bool P2PController::join_swarm_success(QString swarm_hash) {
  return envs_joined_swarm_hash.find(swarm_hash) != envs_joined_swarm_hash.end();
}

bool P2PController::handshake_success(QString env_id, QString cont_id) {
  return successfull_handshakes.find(std::make_pair(env_id, cont_id)) != successfull_handshakes.end();
}

void P2PController::p2p_restart() {
  envs_joined_swarm_hash.clear();
  successfull_handshakes.clear();
}

/////////////////////////////////////////////////////////////////////////

std::vector<CEnvironment> P2PController::get_envs(bool joined_swarm){
  std::vector<CEnvironment> current_envs = CHubController::Instance().lst_healthy_environments();
  std::vector<CEnvironment> ret_envs;
  for (CEnvironment &env : current_envs) {
    if (join_swarm_success(env.hash())) {
      if(joined_swarm)
        ret_envs.push_back(env);
    }
    else { // not joined
      if(!joined_swarm)
        ret_envs.push_back(env);
    }
  }
  return ret_envs;
}

/////////////////////////////////////////////////////////////////////////

void P2PController::update_join_swarm_status(){
  if (!CSystemCallWrapper::p2p_daemon_check()) {
    return;
  }
  std::vector<CEnvironment> current_envs = get_envs(false);
  qDebug() << "Healthy envs not joined to swarm: " << current_envs.size();
  qDebug() << "All envs count: "<< (CHubController::Instance().lst_environments()).size();

  if (!current_envs.empty()){
    for (CEnvironment env : current_envs) {
      join_swarm(env);
    }
  }
}

/////////////////////////////////////////////////////////////////////////

void P2PController::update_handshake_status() {
  if (!CSystemCallWrapper::p2p_daemon_check()) {
    return;
  }

  std::vector<CEnvironment> current_envs = get_envs(true);
  if (!current_envs.empty()) {
    qDebug() << "Create Handshaker instance and move it to thread";
    QThread* thread = new QThread();
    HandshakeSender* handshake_sender = new HandshakeSender(current_envs);
    handshake_sender->moveToThread(thread);
    connect(thread, &QThread::started, handshake_sender, &HandshakeSender::send_handshakes);
    connect(handshake_sender, &HandshakeSender::handshake_success,
            this, &P2PController::handshaked);
    connect(handshake_sender, &HandshakeSender::handshake_failure,
            this, &P2PController::handshake_failed);
    connect(handshake_sender, &HandshakeSender::sent_handshakes_succsessfully, thread, &QThread::quit);
    connect(thread, &QThread::finished, handshake_sender, &HandshakeSender::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
  }
}

/////////////////////////////////////////////////////////////////////////

int P2PController::get_container_status(const CEnvironment *env,
                                               const CHubContainer *cont){

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
