#include <QDir>
#include <QThread>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>

#include "RestWorker.h"
#include "HubController.h"
#include "NotifiactionObserver.h"
#include "SystemCallWrapper.h"
#include "ApplicationLog.h"
#include "SettingsManager.h"
#define UNDEFINED_BALANCE "Undefined balance"

CHubController::CHubController() :
  m_lst_environments_internal(),
  m_lst_resource_hosts(),
  m_balance(UNDEFINED_BALANCE)
{
  connect(&m_refresh_timer, SIGNAL(timeout()),
          this, SLOT(refresh_timer_timeout()));
  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec() * 1000);
  m_refresh_timer.start();

  connect(&m_report_timer, SIGNAL(timeout()), this, SLOT(report_timer_timeout()));
  m_report_timer.setInterval(60*1000); //minute
  m_report_timer.start();
}

CHubController::~CHubController() {
//  we can free p2p resources. but we won't do it.
//  for (auto i = m_lst_environments_internal.begin(); i != m_lst_environments_internal.end(); ++i) {
//    CSystemCallWrapper::leave_p2p_swarm(i->hash().toStdString().c_str());
//  }
}

void
CHubController::ssh_to_container_internal(const CEnvironmentEx *env,
                                          const CHubContainerEx *cont,
                                          void *additional_data,
                                          finished_slot_t slot) {
  std::string rh_ip = cont->rh_ip().toStdString();
  if (rh_ip.empty()) {
    CApplicationLog::Instance()->LogError("Resourse host IP is empty. Conteiner ID : %s",
                                          cont->id().toStdString().c_str());
    emit ssh_to_container_finished(SLE_CONT_NOT_READY, additional_data);
    return;
  }

  CHubControllerP2PWorker* th_worker =
      new CHubControllerP2PWorker(env->hash().toStdString(),
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
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container_finished_str_slot(int result,
                                                   void *additional_data) {
  emit ssh_to_container_str_finished(result, additional_data);
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::refresh_timer_timeout() {
  refresh_environments_res_t rr = RER_ERROR;
  refresh_containers_internal();
  if ((rr=refresh_environments_internal()) == RER_NO_DIFF)
    return;
  emit environments_updated((int)rr);

  if (refresh_balance() == 0)
    emit balance_updated();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::settings_changed() {
  m_refresh_timer.stop();
  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec() * 1000);
  m_refresh_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::report_timer_timeout() {
  m_report_timer.stop();
  int http_code, err_code, network_err;
  QString p2p_version, p2p_status;
  CSystemCallWrapper::p2p_version(p2p_version);
  CSystemCallWrapper::p2p_status(p2p_status);
  CRestWorker::Instance()->send_health_request(http_code, err_code, network_err, p2p_version, p2p_status);
  m_report_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::force_refresh() {
  QtConcurrent::run(this, &CHubController::refresh_timer_timeout);
}
////////////////////////////////////////////////////////////////////////////

int
CHubController::refresh_balance() {
  int http_code, err_code, network_error;
  CHubBalance balance = CRestWorker::Instance()->get_balance(http_code, err_code, network_error);

  if (err_code == RE_NOT_JSON_DOC) {
    CApplicationLog::Instance()->LogInfo("Failed to refresh balance. Received not json. Trying to re-login");
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass,
                                   lhttp, lerr, lnet);
    if (lerr == RE_SUCCESS) {
      CApplicationLog::Instance()->LogInfo("Re-login successful. Trying to get balance again");
      balance = CRestWorker::Instance()->get_balance(http_code, err_code, network_error);
      CApplicationLog::Instance()->LogInfo("%d - %d - %d", http_code, err_code, network_error);
    } else {
      CApplicationLog::Instance()->LogInfo("Failed to re-login. %d - %d - %d", lhttp, lerr, lnet);
    }
  }

  m_balance = err_code ? QString(UNDEFINED_BALANCE) : QString("Balance: %1").arg(balance.value());
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void CHubController::refresh_containers_internal() {
  int http_code, err_code, network_error;
  std::vector<CRHInfo> res = CRestWorker::Instance()->get_ssh_containers(http_code, err_code, network_error);

  if (err_code == RE_NOT_JSON_DOC) {
    CApplicationLog::Instance()->LogInfo("Failed to refresh containers. Received not json. Trying to re-login");
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass,
                                   lhttp, lerr, lnet);
    if (lerr == RE_SUCCESS) {
      res = CRestWorker::Instance()->get_ssh_containers(http_code, err_code, network_error);
    } else {
      CApplicationLog::Instance()->LogInfo("Failed to re-login. %d - %d - %d", lhttp, lerr, lnet);
    }
  }

  if (err_code) {
    CApplicationLog::Instance()->LogError(
          "Refresh containers info error : %s", CRestWorker::rest_err_to_str((rest_error_t)err_code).toStdString().c_str());
    return;
  }

  if (network_error != 0) {    
    CApplicationLog::Instance()->LogError("Refresh containers network error : %d", network_error);
    return;
  }

  if (res == m_lst_resource_hosts)
    return;

  {
    SynchroPrimitives::Locker lock(&m_refresh_cs);
    m_lst_resource_hosts = std::move(res);
  }
}
////////////////////////////////////////////////////////////////////////////

CHubController::refresh_environments_res_t
CHubController::refresh_environments_internal() {

  int http_code, err_code, network_error;
  std::vector<CEnvironment> res;
  res = CRestWorker::Instance()->get_environments(http_code, err_code, network_error);

  if (err_code == RE_NOT_JSON_DOC) {
    CApplicationLog::Instance()->LogInfo("Failed to refresh environments. Received not json. Trying to re-login");
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass,
                                   lhttp, lerr, lnet);
    if (lerr == RE_SUCCESS) {
      res = CRestWorker::Instance()->get_environments(http_code, err_code, network_error);
    } else {
      CApplicationLog::Instance()->LogInfo("Failed to re-login. %d - %d - %d", lhttp, lerr, lnet);
    }
  }

  if (err_code || network_error) {
    CApplicationLog::Instance()->LogError("Refresh environments failed. Err_code : %d, Net_err : %d",
                                          err_code, network_error);
  }

  //that means that about network error notified RestWorker.
  if (err_code && err_code != RE_NETWORK_ERROR) {
    QString err_msg = QString("Refresh environments error : %1").
                      arg(CRestWorker::rest_err_to_str((rest_error_t)err_code));
    CNotificationObserver::NotifyAboutError(err_msg);
    return RER_ERROR;
  }

  if (network_error != 0)
    return RER_ERROR;

  if (res == m_lst_environments_internal)
    return m_lst_environments_internal.empty() ? RER_EMPTY : RER_NO_DIFF;

  {
    SynchroPrimitives::Locker lock(&m_refresh_cs);
    m_lst_environments_internal = std::move(res);
    m_lst_environments.erase(m_lst_environments.begin(), m_lst_environments.end());
    for (auto env = m_lst_environments_internal.cbegin(); env != m_lst_environments_internal.cend(); ++env) {
      m_lst_environments.push_back(CEnvironmentEx(*env, m_lst_resource_hosts));
    }
  }
  return RER_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container(const CEnvironmentEx *env,
                                 const CHubContainerEx *cont,
                                 void* additional_data) {

  ssh_to_container_internal(env, cont, additional_data, ssh_to_cont);
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container_str(const QString &env_id,
                                     const QString &cont_id,
                                     void *additional_data) {
  refresh_environments_internal();

  CEnvironmentEx *env = NULL;
  const CHubContainerEx *cont = NULL;

  {
    SynchroPrimitives::Locker lock(&m_refresh_cs);
    for (auto i = m_lst_environments.begin(); i != m_lst_environments.end(); ++i) {
      if (i->id() != env_id) continue;
      env = &(*i); break;
    }
    if (env == NULL) {
      emit ssh_to_container_finished(SLE_ENV_NOT_FOUND, additional_data);
      return;
    }

    for (auto j = env->containers().begin(); j != env->containers().end(); ++j) {
      if (j->id() != cont_id) continue;
      cont = &(*j); break;
    }
    if (cont == NULL) {
      emit ssh_to_container_finished(SLE_CONT_NOT_FOUND, additional_data);
      return;
    }
  }

  ssh_to_container_internal(env, cont, additional_data, ssh_to_cont_str);
}
////////////////////////////////////////////////////////////////////////////

const QString&
CHubController::ssh_launch_err_to_str(int err) {
  static QString lst_err_str[SLE_LAST_ERR] = {
    "Success",
    "Environment not found",
    "Container not found",
    "Container isn't ready",
    "Join to p2p swarm failed",
    "System call failed" };
  return lst_err_str[err%SLE_LAST_ERR];
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CHubControllerP2PWorker::CHubControllerP2PWorker(const std::string &env_hash,
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

CHubControllerP2PWorker::~CHubControllerP2PWorker()
{

}
////////////////////////////////////////////////////////////////////////////

void
CHubControllerP2PWorker::join_to_p2p_swarm_begin() {
  system_call_wrapper_error_t err = CSystemCallWrapper::join_to_p2p_swarm(m_env_hash.c_str(),
                                                                          m_env_key.c_str(),
                                                                          "dhcp");
  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Failed to join to p2p network. Error : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotificationObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    emit join_to_p2p_swarm_finished((int)SLE_JOIN_TO_SWARM_FAILED);
    return;
  }

  emit join_to_p2p_swarm_finished((int)SLE_SUCCESS);
}
////////////////////////////////////////////////////////////////////////////

void
CHubControllerP2PWorker::ssh_to_container_begin(int join_result) {
  system_call_wrapper_error_t err;

  if (join_result != SLE_SUCCESS) {
    emit ssh_to_container_finished(join_result, m_additional_data);
    return;
  }

  CNotificationObserver::NotifyAboutInfo("Checking container. Please, wait");
  static const int MAX_ATTEMTS_COUNT = 25;
  for (int ac = 0; ac < MAX_ATTEMTS_COUNT; ++ac) {
    err = CSystemCallWrapper::check_container_state(m_env_hash.c_str(),
                                                    m_ip.c_str());
    if (err == SCWE_SUCCESS) break;
    QThread::currentThread()->sleep(1);
  }

  if (err != SCWE_SUCCESS) {
    CNotificationObserver::NotifyAboutError("Failed to run SSH because container isn't ready. Try little bit later.");
    emit ssh_to_container_finished((int)SLE_CONT_NOT_READY, m_additional_data);
    return;
  }

  QFile key_file_pub(CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() +
                     CHubController::Instance().current_user() +
                     QString(".pub"));
  QFile key_file_private(CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() +
                         CHubController::Instance().current_user());
  std::string key;

  if (key_file_pub.exists() &&  key_file_private.exists()) {
    key = (CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() +
           CHubController::Instance().current_user()).toStdString();
  } else {
    QFile standard_key_pub(CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() +
                                "id_rsa.pub");
    QFile standard_key_private(CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() +
                               "id_rsa");

    if (standard_key_pub.exists() && standard_key_private.exists()) {
      key = (CSettingsManager::Instance().ssh_keys_storage() +
            QDir::separator() + "id_rsa").toStdString();
    }
  }

  err = CSystemCallWrapper::run_ssh_in_terminal(CSettingsManager::Instance().ssh_user(),
                                                QString(m_ip.c_str()),
                                                QString(m_cont_port.c_str()), //todo change to QString!!!
                                                key.empty() ? NULL : key.c_str());

  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Run SSH failed. Error code : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotificationObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    emit ssh_to_container_finished((int)SLE_SYSTEM_CALL_FAILED, m_additional_data);
    return ;
  }

  emit ssh_to_container_finished((int)SLE_SUCCESS, m_additional_data);
}
////////////////////////////////////////////////////////////////////////////

CHubContainerEx::CHubContainerEx(const CHubContainer &cont,
                                 const std::vector<CRHInfo>& lst_resource_hosts) :
  m_name(cont.name()), m_ip(cont.ip()),
  m_id(cont.id()), m_port(cont.port()), m_rh_ip(QString()) {

  bool found = false;
  for (auto rh = lst_resource_hosts.begin(); !found && rh != lst_resource_hosts.end(); ++rh) {
    for (auto rh_cont = rh->lst_containers().begin(); rh_cont != rh->lst_containers().end(); ++rh_cont) {
      if (rh_cont->id() != m_id) continue;
      m_rh_ip = rh->rh_ip();
      found = true;
      break;
    }
  }
}
////////////////////////////////////////////////////////////////////////////

CEnvironmentEx::CEnvironmentEx(const CEnvironment &env,
                               const std::vector<CRHInfo>& lst_resource_hosts) :
  m_name(env.name()), m_hash(env.hash()), m_aes_key(env.key()),
  m_ttl(env.ttl()), m_id(env.id()), m_status(env.status()),
  m_status_descr(env.status_description()) {

  for (auto cont = env.containers().cbegin(); cont != env.containers().cend(); ++cont) {
    m_lst_containers.push_back( CHubContainerEx(*cont, lst_resource_hosts));
  }
}
////////////////////////////////////////////////////////////////////////////
