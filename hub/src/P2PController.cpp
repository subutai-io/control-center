#include "P2PController.h"
#include <QThread>
#include "Locker.h"

static SynchroPrimitives::CriticalSection handshake_cs;



/////////////////////////////////////////////////////////////////////////

SwarmConnector::~SwarmConnector() {
  m_th->quit();
}

SwarmLeaver::~SwarmLeaver() {
  m_th->quit();
}

/////////////////////////////////////////////////////////////////////////

SwarmConnector::SwarmConnector(QString swarm_hash, QString swarm_key)
  : swarm_hash(swarm_hash) , swarm_key(swarm_key) {
  m_th = new QThread;
  connect(m_th, &QThread::started, this, &SwarmConnector::join_to_swarm_begin);
  connect(this, &SwarmConnector::join_to_swarm_finished, m_th, &QThread::quit);
  connect(m_th, &QThread::finished, this, &SwarmConnector::deleteLater);
  connect(m_th, &QThread::finished, m_th, &QThread::deleteLater);
  moveToThread(m_th);
  m_th->start();
}

SwarmLeaver::SwarmLeaver(QString swarm_hash, QString swarm_key) : swarm_hash(swarm_hash), swarm_key(swarm_key) {
  m_th = new QThread;
  connect(m_th, &QThread::started, this, &SwarmLeaver::leave_swarm_begin);
  connect(this, &SwarmLeaver::leave_swarm_finished, m_th, &QThread::quit);
  connect(m_th, &QThread::finished, this, &SwarmConnector::deleteLater);
  connect(m_th, &QThread::finished, m_th, &QThread::deleteLater);
  moveToThread(m_th);
  m_th->start();
}

///////////////////////////////////////////////////////////////

void SwarmConnector::join_to_swarm_begin() {
  system_call_wrapper_error_t res = CSystemCallWrapper::join_to_p2p_swarm(swarm_hash, swarm_key, "dhcp");
  if (res == SCWE_SUCCESS)
    emit successfully_joined_swarm(swarm_hash);
  emit join_to_swarm_finished();
}

void SwarmLeaver::leave_swarm_begin() {
  system_call_wrapper_error_t res = CSystemCallWrapper::leave_p2p_swarm(swarm_hash);
  if (res == SCWE_SUCCESS)
    emit successfully_left_swarm(swarm_hash);
  emit leave_swarm_finished();
}

/////////////////////////////////////////////////////////////////////////

void HandshakeSender::try_to_handshake(const CEnvironment &env, const CHubContainer &cont) {
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

  if (err == SCWE_SUCCESS)
    emit handshake_success(env.id(), cont.id());
  else
    emit handshake_failure(env.id(), cont.id());
}

void HandshakeSender::send_handshakes(){
  SynchroPrimitives::Locker lock(&handshake_cs); // locker for threads
  for (CEnvironment &env : m_envs) {
    for (CHubContainer cont : env.containers()) {
      try_to_handshake(env, cont);
    }
  }
  emit sent_handshakes_succsessfully();
}


HandshakeSender::HandshakeSender(const std::vector<CEnvironment> envs) : m_envs(envs) {
  m_th = new QThread;
  connect(m_th, &QThread::started, this, &HandshakeSender::send_handshakes);
  connect(this, &HandshakeSender::sent_handshakes_succsessfully, m_th, &QThread::quit);
  connect(m_th, &QThread::finished, this, &HandshakeSender::deleteLater);
  connect(m_th, &QThread::finished, m_th, &QThread::deleteLater);
  moveToThread(m_th);
  m_th->start();
}
HandshakeSender::~HandshakeSender(){
 m_th->quit();
}
/////////////////////////////////////////////////////////////////////////


P2PController::P2PController() {
  QTimer *join_to_swarm_timer = new QTimer(this);
  connect(&EnvironmentState::Instance(), &EnvironmentState::environemts_state_updated,
          this, &P2PController::update_handshake_status);
  connect(join_to_swarm_timer, &QTimer::timeout,
          this, &P2PController::update_join_swarm_status);
  join_to_swarm_timer->start(1000 * 60 * 4); // 4 minutes
}

/////////////////////////////////////////////////////////////////////////

void P2PController::joined_swarm(QString hash) {
  envs_joined_swarm_hash.insert(hash);
  // qInfo() << "Successfully joined the swarm : " << hash;
}
#include <QCoreApplication>

void P2PController::join_swarm(const CEnvironment &env) {
  //qInfo() << "Trying to  join the swarm for environment " << env.name()
          // << " with hash: " << env.hash() << " and key: " << env.key();
  SwarmConnector *connector = new SwarmConnector(env.hash(), env.key());
  connect(connector, &SwarmConnector::successfully_joined_swarm,
          this, &P2PController::joined_swarm);
}

/////////////////////////////////////////////////////////////////////////

void P2PController::left_swarm(QString hash) {
  envs_joined_swarm_hash.erase(hash);
}

void P2PController::leave_swarm(const CEnvironment &env) {
  SwarmLeaver *leaver = new SwarmLeaver(env.hash(), env.key());
  connect(leaver, &SwarmLeaver::successfully_left_swarm,
          this, &P2PController::left_swarm);
}

/////////////////////////////////////////////////////////////////////////



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
  for (CEnvironment &env : disconnected_envs) {
    if (join_swarm_success(env.hash()))
      leave_swarm(env);
  }
}

void P2PController::update_join_swarm_status(){
  std::vector<CEnvironment> current_envs = EnvironmentState::Instance().last_updated_envs(); // last_updated_envs stores both envs that are freshly builded and not old not deleted envs
  for (CEnvironment &env : current_envs) {
    if (!join_swarm_success(env.hash()))
      join_swarm(env);
  }
}

/////////////////////////////////////////////////////////////////////////

void P2PController::update_handshake_status() {
  if (!EnvironmentState::Instance().connected_envs().empty()) {
    update_join_swarm_status();
  }

  if (!EnvironmentState::Instance().disconnected_envs().empty()) {
    update_leave_swarm_status();
  }

  HandshakeSender *handshake_sender = new HandshakeSender(get_joined_envs());
  connect(handshake_sender, &HandshakeSender::handshake_success,
          this, &P2PController::handshaked);
  connect(handshake_sender, &HandshakeSender::handshake_failure,
          this, &P2PController::handshake_failed);

}

/////////////////////////////////////////////////////////////////////////
