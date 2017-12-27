#include "P2PController.h"
#include <QThread>
#include "Locker.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>



/////////////////////////////////////////////////////////////////////////

SwarmConnector::SwarmConnector(const CEnvironment &_env, QObject* parent)
  : QObject(parent), env(_env) {
  qDebug() << QString("SwarmConnector with [env_name: %1, env_id: %2, swarm_hash: %3] and key: %4")
                .arg(env.name())
                .arg(env.id())
                .arg(env.hash())
                .arg(env.key());

  attemptCounter = 0;
}
/////////////////////////////////////////////////////////////////////////

void SwarmConnector::join_to_swarm_begin() {
  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::join_to_p2p_swarm, env.hash(), env.key(), QString("dhcp"));
  res.waitForFinished();

  if (res.result() == SCWE_SUCCESS) {
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
                  .arg(CSystemCallWrapper::scwe_error_to_str(res.result()));
    attemptCounter ++;
    if(attemptCounter < 10) {
      QTimer::singleShot((int) std::pow(2, attemptCounter) * 1000, this, &SwarmConnector::join_to_swarm_begin); // after each attempt, it will increate its interval
    }
    else {
      emit join_swarm_timeout(env);
    }
  }
}
/////////////////////////////////////////////////////////////////////////

HandshakeSender::HandshakeSender(const CEnvironment &_env, QObject *parent)
  : env(_env), QObject(parent) {
  QTimer *timer = new QTimer;
  timer->setInterval(1000 * 15); // 15 sec
  connect(timer, &QTimer::timeout, this, &HandshakeSender::handshake_begin);
  timer->start();
  //this->setAttribute(Qt::WA_DeleteOnClose);
  //this->setParent(this);

}
/////////////////////////////////////////////////////////////////////////

void HandshakeSender::try_to_handshake(const CHubContainer &cont) {
  qDebug() << QString("Trying to handshake with container: [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]")
              .arg(cont.name())
              .arg(cont.id())
              .arg(env.name())
              .arg(env.id())
              .arg(env.hash());

  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::check_container_state, env.hash(), cont.rh_ip());
  res.waitForFinished();

  if (res.result() == SCWE_SUCCESS)
  {
      res = QtConcurrent::run(CSystemCallWrapper::send_handshake, cont.rh_ip(), cont.port());
      res.waitForFinished();
  }

  if (res.result() == SCWE_SUCCESS) {
    qInfo() << QString("Successfully handshaked with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]")
               .arg(cont.name())
               .arg(cont.id())
               .arg(env.name())
               .arg(env.id())
               .arg(env.hash());
    emit handshake_success(env.id(), cont.id());
  }
  else {
    qInfo() << QString("Can't handshake with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]. Error message: %6")
               .arg(cont.name())
               .arg(cont.id())
               .arg(env.name())
               .arg(env.id())
               .arg(env.hash())
               .arg(CSystemCallWrapper::scwe_error_to_str(res.result()));
    emit handshake_failure(env.id(), cont.id());
  }
}
/////////////////////////////////////////////////////////////////////////

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
  m_join_to_swarm_timer->start(1000 * 60 * 3); // 3 minutes
  QTimer::singleShot(10000, this, &P2PController::update_join_swarm_status); // 10 sec
}
/////////////////////////////////////////////////////////////////////////

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
/////////////////////////////////////////////////////////////////////////

bool P2PController::handshake_success(QString env_id, QString cont_id) {
  return successfull_handshakes.find(std::make_pair(env_id, cont_id)) != successfull_handshakes.end();
}
/////////////////////////////////////////////////////////////////////////

void P2PController::handshake_failed(QString env_id, QString cont_id) {
  successfull_handshakes.erase(std::make_pair(env_id, cont_id));
}
/////////////////////////////////////////////////////////////////////////

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

void P2PController::remove_connector(const CEnvironment &env) {
 qCritical() << QString("Cannot connect to swarm for [env_name: %1, env_id: %2, swarm_hash: %3]")
                .arg(env.name())
                .arg(env.id())
                .arg(env.hash());
 envs_with_connectors.erase(env.id());
}
/////////////////////////////////////////////////////////////////////////

bool P2PController::has_connector(QString env_id) {
  return envs_with_connectors.find(env_id) != envs_with_connectors.end();
}
/////////////////////////////////////////////////////////////////////////

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
/////////////////////////////////////////////////////////////////////////

void P2PController::joined_swarm(const CEnvironment &env) {
  envs_joined_swarm_hash.insert(env.hash());
}
/////////////////////////////////////////////////////////////////////////

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

  connect(connector, &SwarmConnector::join_swarm_timeout, this, &P2PController::remove_connector);
  connect(connector, &SwarmConnector::join_swarm_timeout, thread, &QThread::quit);

  connect(thread, &QThread::finished, connector, &SwarmConnector::deleteLater);
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();

  envs_with_connectors.insert(env.id());
}
/////////////////////////////////////////////////////////////////////////

void P2PController::update_join_swarm_status(){
  if (!CSystemCallWrapper::p2p_daemon_check()) {
    envs_joined_swarm_hash.clear();
    qDebug() << "Trying to start p2p daemon.";
    int rse_err = 0;
    system_call_wrapper_error_t scwe =
        CSystemCallWrapper::restart_p2p_service(&rse_err);

    if (scwe != SCWE_SUCCESS) {
      qCritical() << "Can't restart p2p daemon.";
    } else if (scwe == SCWE_SUCCESS) {
      qInfo() << "P2p daemon successfully restarted.";
    }
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
/////////////////////////////////////////////////////////////////////////

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
/////////////////////////////////////////////////////////////////////////

p2p_message_res_t P2PController::status(const CEnvironment *env, const CHubContainer *cont) {
  p2p_message_res_t res;

  if (!env->healthy()) {
    res.btn_ssh_message = res.btn_desktop_message = QString("%1. Environment status: %2").arg(env->status_description()).arg(env->status());
    res.btn_ssh_enabled = res.btn_desktop_enabled = false;
    return res;
  }
  else
  if(!env || !P2PController::Instance().join_swarm_success(env->hash())) {
    res.btn_desktop_message = res.btn_ssh_message = p2p_messages[CANT_JOIN_SWARM];
    res.btn_ssh_enabled = res.btn_desktop_enabled = false;
    return res;
  }
  else
  if (!cont || !P2PController::Instance().handshake_success(env->id(), cont->id())) {
    res.btn_desktop_message = res.btn_ssh_message = p2p_messages[CANT_CONNECT_CONTAINER];
    res.btn_ssh_enabled = res.btn_desktop_enabled = false;
    return res;
  }
  else {
    res.btn_desktop_message = p2p_messages[CLICK_EZ_DESKTOP];
    res.btn_ssh_message = p2p_messages[CLICK_EZ_SSH];
    res.btn_ssh_enabled = res.btn_desktop_enabled = true;
    return res;
  }
}
