#include <QDir>
#include <QThread>
#include <QApplication>

#include "RestWorker.h"
#include "HubController.h"
#include "NotifiactionObserver.h"
#include "SystemCallWrapper.h"
#include "ApplicationLog.h"
#include "SettingsManager.h"
#define UNDEFINED_BALANCE "Undefined balance"

CHubController::CHubController() :
  m_lst_environments(),
  m_lst_resource_hosts(),
  m_balance(UNDEFINED_BALANCE)
{
}

CHubController::~CHubController() {
}

void
CHubController::ssh_to_container_internal(const CSSEnvironment *env,
                                          const CHubContainer *cont,
                                          void *additional_data,
                                          finished_slot_t slot) {
  std::string rh_ip;
  {
    SynchroPrimitives::Locker lock(&m_refresh_cs);
    for (auto rh = m_lst_resource_hosts.begin(); rh != m_lst_resource_hosts.end(); ++rh) {
      for (auto rh_cont = rh->lst_containers().begin(); rh_cont != rh->lst_containers().end(); ++rh_cont) {
        if (rh_cont->id() != cont->id()) continue;
        rh_ip = rh->rh_ip().toStdString();
        break;
      }
    }
    if (rh_ip.empty()) {
      emit ssh_to_container_finished(SLE_CONT_NOT_FOUND, additional_data);
      return;
    }
  }

  CHubControllerThreadWorker* th_worker =
      new CHubControllerThreadWorker(env->hash().toStdString(),
                                     env->key().toStdString(),
                                     rh_ip,
                                     cont->port().toStdString(),
                                     additional_data);

  QThread* th = new QThread;
  connect(th, SIGNAL(started()), th_worker, SLOT(join_to_p2p_swarm_begin()));
  connect(th_worker, SIGNAL(join_to_p2p_swarm_finished(int)), th_worker, SLOT(ssh_to_container_begin(int)));

  /*hack, but I haven't enough time*/
  if (slot == ssh_to_cont)
    connect(th_worker, SIGNAL(ssh_to_container_finished(int, void*)), this, SLOT(ssh_to_container_finished_slot(int, void*)));
  else if (slot == ssh_to_cont_str)
    connect(th_worker, SIGNAL(ssh_to_container_finished(int,void*)), this, SLOT(ssh_to_container_finished_str_slot(int,void*)));

  connect(th_worker, SIGNAL(ssh_to_container_finished(int, void*)), th, SLOT(quit()));
  connect(th, SIGNAL(finished()), th_worker, SLOT(deleteLater()));
  connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));

  th_worker->moveToThread(th);
  th->start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container_finished_slot(int result, void *additional_data) {
  emit ssh_to_container_finished(result, additional_data);
}

void
CHubController::ssh_to_container_finished_str_slot(int result,
                                                   void *additional_data) {
  emit ssh_to_container_str_finished(result, additional_data);
}
////////////////////////////////////////////////////////////////////////////

int CHubController::refresh_balance() {
  int http_code, err_code, network_error;
  CSSBalance balance = CRestWorker::Instance()->get_balance(http_code, err_code, network_error);
  m_balance = err_code ? QString(UNDEFINED_BALANCE) : QString("Balance:  %1").arg(balance.value());
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CHubController::refresh_environments() {

  int http_code, err_code, network_error;
  std::vector<CSSEnvironment> res = CRestWorker::Instance()->get_environments(http_code, err_code, network_error);

  if (err_code) {
    QString err_msg = QString("Refresh environments error : %1").
                      arg(CRestWorker::login_err_to_str((error_login_t)err_code));
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError("Refresh environments error : %d", err_code);
    return 1;
  }

  if (network_error != 0) {
    QString err_msg = QString("Refresh environments network error : %1").arg(network_error);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    return 1;
  }

  if (res == m_lst_environments)
    return 1;
  SynchroPrimitives::Locker lock(&m_refresh_cs);
  m_lst_environments = std::move(res);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void CHubController::refresh_containers() {
  int http_code, err_code, network_error;
  std::vector<CRHInfo> res = CRestWorker::Instance()->get_ssh_containers(http_code, err_code, network_error);

  if (err_code) {
    QString err_msg = QString("Refresh containers info error : %1").
                      arg(CRestWorker::login_err_to_str((error_login_t)err_code));
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    return;
  }

  if (network_error != 0) {
    QString err_msg = QString("Refresh containers network error : %1").arg(network_error);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    return;
  }

  if (res == m_lst_resource_hosts)
    return;
  SynchroPrimitives::Locker lock(&m_refresh_cs);
  m_lst_resource_hosts = std::move(res);
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container(const CSSEnvironment *env,
                                 const CHubContainer *cont,
                                 void* additional_data) {

  ssh_to_container_internal(env, cont, additional_data, ssh_to_cont);
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container_str(const QString &env_id,
                                     const QString &cont_id,
                                     void *additional_data) {
  SynchroPrimitives::Locker lock(&m_refresh_cs);
  CSSEnvironment *env = NULL;
  for (auto i = m_lst_environments.begin(); i != m_lst_environments.end(); ++i) {
    if (i->id() != env_id) continue;
    env = &(*i); break;
  }
  if (env == NULL) {
    emit ssh_to_container_finished(SLE_ENV_NOT_FOUND, additional_data);
    return;
  }

  const CHubContainer *cont = NULL;
  for (auto j = env->containers().begin(); j != env->containers().end(); ++j) {
    if (j->id() != cont_id) continue;
    cont = &(*j); break;
  }
  if (cont == NULL) {
    emit ssh_to_container_finished(SLE_CONT_NOT_FOUND, additional_data);
    return;
  }
  ssh_to_container_internal(env, cont, additional_data, ssh_to_cont_str);
}
////////////////////////////////////////////////////////////////////////////

const QString&
CHubController::ssh_launch_err_to_str(int err) {
  static QString lst_err_str[] = {
    "Success", "Environment not found", "Container not found", "Join to p2p swarm failed", "System call failed" };
  return lst_err_str[err%SLE_LAST_ERR];
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CHubControllerThreadWorker::CHubControllerThreadWorker(const std::string &env_hash,
                                                       const std::string &env_key,
                                                       const std::string &ip,
                                                       const std::string &cont_port,
                                                       void *additional_data) :
  m_env_hash(env_hash),
  m_env_key(env_key),
  m_ip(ip),
  m_cont_port(cont_port),
  m_additional_data(additional_data)
{

}

CHubControllerThreadWorker::~CHubControllerThreadWorker()
{

}
////////////////////////////////////////////////////////////////////////////

void
CHubControllerThreadWorker::join_to_p2p_swarm_begin() {
  system_call_wrapper_error_t err = CSystemCallWrapper::join_to_p2p_swarm(m_env_hash.c_str(),
                                                                          m_env_key.c_str(),
                                                                          m_ip.c_str());
  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Failed to join to p2p network. Error : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    emit join_to_p2p_swarm_finished((int)SLE_JOIN_TO_SWARM_FAILED);
    return;
  }

  emit join_to_p2p_swarm_finished((int)SLE_SUCCESS);
}
////////////////////////////////////////////////////////////////////////////

void
CHubControllerThreadWorker::ssh_to_container_begin(int join_result) {
  system_call_wrapper_error_t err;

  if (join_result != SLE_SUCCESS) {
    emit ssh_to_container_finished(join_result, m_additional_data);
    return;
  }

  static const int MAX_ATTEMTS_COUNT = 5;
  for (int ac = 0; ac < MAX_ATTEMTS_COUNT; ++ac) {
    err = CSystemCallWrapper::check_container_state(m_env_hash.c_str(),
                                                    m_ip.c_str());
    if (err == SCWE_SUCCESS) break;
    QThread::currentThread()->sleep(1);
  }

  if (err != SCWE_SUCCESS) {
    CNotifiactionObserver::NotifyAboutError("Failed to run SSH because container isn't ready. Try little bit later.");
    emit ssh_to_container_finished((int)err, m_additional_data);
    return;
  }

  QFile key_file_pub(QApplication::applicationDirPath() + QDir::separator() +
                     CHubController::Instance().current_user() +
                     QString(".pub"));
  QFile key_file_private(QApplication::applicationDirPath() + QDir::separator() +
                         CHubController::Instance().current_user());
  std::string key;

  if (key_file_pub.exists() &&  key_file_private.exists()) {
    key = (QApplication::applicationDirPath() + QDir::separator() +
           CHubController::Instance().current_user()).toStdString();
  }

  err = CSystemCallWrapper::run_ssh_in_terminal(CSettingsManager::Instance().ssh_user().toStdString().c_str(),
                                                m_ip.c_str(),
                                                m_cont_port.c_str(),
                                                key.empty() ? NULL : key.c_str());

  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Run SSH failed. Error code : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    emit ssh_to_container_finished((int)SLE_SYSTEM_CALL_FAILED, m_additional_data);
    return ;
  }

  emit ssh_to_container_finished((int)SLE_SUCCESS, m_additional_data);
}
////////////////////////////////////////////////////////////////////////////
