#include "P2PController.h"
#include <QThread>
#include "Locker.h"
#include <QDebug>



/////////////////////////////////////////////////////////////////////////

SwarmConnector::~SwarmConnector() {
  qDebug() << QString("SwarmConnector desctructor");
}

SwarmLeaver::~SwarmLeaver() {
  qDebug() << QString("SwarmLeaver desctructor");
}

/////////////////////////////////////////////////////////////////////////

SwarmConnector::SwarmConnector(QString swarm_hash, QString swarm_key) : swarm_hash(swarm_hash) , swarm_key(swarm_key) {
  qDebug() << QString("SwarmConnector with hash: %1 and key: %2").arg(swarm_hash).arg(swarm_key);
}

SwarmLeaver::SwarmLeaver(QString swarm_hash, QString swarm_key) : swarm_hash(swarm_hash), swarm_key(swarm_key) {
  qDebug() << QString("SwarmLeaver with hash: %1 and key: %2").arg(swarm_hash).arg(swarm_key);
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

void SwarmLeaver::leave_swarm_begin() {
  system_call_wrapper_error_t res = CSystemCallWrapper::leave_p2p_swarm(swarm_hash);
  if (res == SCWE_SUCCESS) {
    qInfo("Left the swarm %s.",
              swarm_hash.toStdString().c_str());
    emit successfully_left_swarm(swarm_hash);
  }
  else {
    qCritical("Can't leave the swarm %s. Err %s",
              swarm_hash.toStdString().c_str(),
              CSystemCallWrapper::scwe_error_to_str(res).toStdString().c_str());
  }

  emit leave_swarm_finished();
}

/////////////////////////////////////////////////////////////////////////

void HandshakeSender::try_to_handshake(const CEnvironment &env, const CHubContainer &cont) {
  qDebug() << QString("Trying to handshake with cont: %1 and env: %2").arg(cont.name()).arg(env.name());

  system_call_wrapper_error_t err =
      CSystemCallWrapper::check_container_state(env.hash(), cont.rh_ip());

  if (err == SCWE_SUCCESS)
    err = CSystemCallWrapper::send_handshake(cont.rh_ip(), cont.port());

  if (err != SCWE_SUCCESS){
    qCritical("Cannot handshake with cont %s in environment %s. Err: %s",
              cont.name().toStdString().c_str(),
              env.name().toStdString().c_str(),
              CSystemCallWrapper::scwe_error_to_str(err).toStdString().c_str());
  }

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
  send_handshakes();
}

HandshakeSender::~HandshakeSender() {}
/////////////////////////////////////////////////////////////////////////


P2PController::P2PController() {
  qDebug("P2PController is initialized");

  m_join_to_swarm_timer = new QTimer(this);
  m_handshake_timer = new QTimer(this);

  connect(m_join_to_swarm_timer, &QTimer::timeout,
          this, &P2PController::update_join_swarm_status);
  connect(m_handshake_timer, &QTimer::timeout,
          this, &P2PController::update_handshake_status);

  m_join_to_swarm_timer->start(1000 * 60 * 3); // 3 minutes
  m_handshake_timer->start(1000 * 60 * 1); // 1 minute
}

/////////////////////////////////////////////////////////////////////////

void P2PController::joined_swarm(QString hash) {
  envs_joined_swarm_hash.insert(hash);
}

void P2PController::join_swarm(const CEnvironment &env) {
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

void P2PController::left_swarm(QString hash) {
  envs_joined_swarm_hash.erase(hash);
}

void P2PController::leave_swarm(const CEnvironment &env) {
  SwarmLeaver *leaver = new SwarmLeaver(env.hash(), env.key());
  QThread* thread = new QThread();
  leaver->moveToThread(thread);

  connect(thread, &QThread::started, leaver, &SwarmLeaver::leave_swarm_begin);
  connect(leaver, &SwarmLeaver::successfully_left_swarm,
          this, &P2PController::left_swarm);
  connect(leaver, &SwarmLeaver::leave_swarm_finished, thread, &QThread::quit);
  connect(thread, &QThread::finished, leaver, &SwarmConnector::deleteLater);
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

std::vector<CEnvironment> P2PController::get_joined_envs(){
  std::vector<CEnvironment> current_envs = EnvironmentState::Instance().last_updated_envs();
  std::vector<CEnvironment> ret_envs;
  for (CEnvironment &env : current_envs) {
    if (join_swarm_success(env.hash())) {
      ret_envs.push_back(env);
    }
  }
  return ret_envs;
}

/////////////////////////////////////////////////////////////////////////

void P2PController::update_leave_swarm_status(){
  std::vector<CEnvironment> disconnected_envs = EnvironmentState::Instance().disconnected_envs();
  qDebug() << "disconnected envs: ";
  for (CEnvironment &env : disconnected_envs) {
    if (join_swarm_success(env.hash()))
      leave_swarm(env);
  }
}

void P2PController::update_join_swarm_status(){
  std::vector<CEnvironment> current_envs = EnvironmentState::Instance().last_updated_envs(); // last_updated_envs stores both envs that are freshly builded and not old not deleted envs
  qDebug() << "Current envs";
  for (CEnvironment &env : current_envs) {
    if (!join_swarm_success(env.hash()))
      join_swarm(env);
  }
}

/////////////////////////////////////////////////////////////////////////

void P2PController::update_handshake_status() {
  qDebug() << "Trying to handshake";

  if (!EnvironmentState::Instance().connected_envs().empty()) {
    update_join_swarm_status();
  }

  if (!EnvironmentState::Instance().disconnected_envs().empty()) {
    update_leave_swarm_status();
  }

  std::vector<CEnvironment> envs = get_joined_envs();
  if (!envs.empty()) {
      QThread* thread = new QThread();
      HandshakeSender* handshake_sender = new HandshakeSender(get_joined_envs());
      handshake_sender->moveToThread(thread);

      connect(thread, &QThread::started, handshake_sender, &HandshakeSender::send_handshakes);
      connect(handshake_sender, &HandshakeSender::handshake_success,
              this, &P2PController::handshaked);
      connect(handshake_sender, &HandshakeSender::handshake_failure,
              this, &P2PController::handshake_failed);
      connect(handshake_sender, &HandshakeSender::sent_handshakes_succsessfully, thread, &QThread::quit);
      connect(handshake_sender, &HandshakeSender::handshake_failure, thread, &QThread::quit);
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
