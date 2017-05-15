#include <QDir>
#include <QThread>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>

#include "RestWorker.h"
#include "HubController.h"
#include "NotificationObserver.h"
#include "SystemCallWrapper.h"
#include "ApplicationLog.h"
#include "SettingsManager.h"
#include "SshKeysController.h"

static const QString undefined_balance("Undefined balance");
static volatile int UPDATED_COMPONENTS_COUNT = 2;

static const int UPDATE_ATTEMPT_MAX = 5;
static volatile int UPDATE_BALANCE_ATTEMPT_COUNT = 0;
static volatile int UPDATE_ENVIRONMENTS_ATTEMTP_COUNT = 0;

CHubController::CHubController() :
  m_lst_environments_internal(),
  m_balance(undefined_balance)
{
  connect(&m_refresh_timer, &QTimer::timeout,
          this, &CHubController::refresh_timer_timeout);
  connect(CRestWorker::Instance(), &CRestWorker::on_get_balance_finished,
          this, &CHubController::on_balance_updated_sl);
  connect(CRestWorker::Instance(), &CRestWorker::on_get_environments_finished,
          this, &CHubController::on_environments_updated_sl);

  connect(&m_report_timer, &QTimer::timeout,
          this, &CHubController::report_timer_timeout);

  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec() * 1000);
  m_refresh_timer.start();

  m_report_timer.setInterval(60*1000); //minute
  m_report_timer.start();
}

CHubController::~CHubController() {
//  we can free p2p resources. but we won't do it.
//  for (auto i = m_lst_environments_internal.begin(); i != m_lst_environments_internal.end(); ++i) {
//    CSystemCallWrapper::leave_p2p_swarm(i->hash());
//  }
}

void
CHubController::ssh_to_container_internal(const CEnvironment *env,
                                          const CHubContainer *cont,
                                          void *additional_data,
                                          finished_slot_t slot) {
  if (cont->rh_ip().isEmpty()) {
    CApplicationLog::Instance()->LogError("Resourse host IP is empty. Conteiner ID : %s",
                                          cont->id().toStdString().c_str());
    emit ssh_to_container_finished(SLE_CONT_NOT_READY, additional_data);
    return;
  }

  CHubControllerP2PWorker* th_worker =
      new CHubControllerP2PWorker(env->id(),
                                  env->hash(),
                                  env->key(),
                                  cont->rh_ip(),
                                  cont->port(),
                                  additional_data);

  QThread* th = new QThread;
  connect(th, &QThread::started,
          th_worker, &CHubControllerP2PWorker::join_to_p2p_swarm_begin);
  connect(th_worker, &CHubControllerP2PWorker::join_to_p2p_swarm_finished,
          th_worker, &CHubControllerP2PWorker::ssh_to_container_begin);

  /*hack, but I haven't enough time*/
  if (slot == ssh_to_cont) {
    connect(th_worker, &CHubControllerP2PWorker::ssh_to_container_finished,
            this, &CHubController::ssh_to_container_finished_slot);
  }
  else if (slot == ssh_to_cont_str) {
    connect(th_worker, &CHubControllerP2PWorker::ssh_to_container_finished,
            this, &CHubController::ssh_to_container_finished_str_slot);
  }

  connect(th_worker, &CHubControllerP2PWorker::ssh_to_container_finished, th, &QThread::quit);
  connect(th, &QThread::finished, th_worker, &CHubControllerP2PWorker::deleteLater);
  connect(th, &QThread::finished, th, &QThread::deleteLater);

  th_worker->moveToThread(th);
  th->start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::refresh_environments_internal() {
  CRestWorker::Instance()->update_environments();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::refresh_balance_internal() {
  CRestWorker::Instance()->update_balance();
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
  m_refresh_timer.stop();
  UPDATED_COMPONENTS_COUNT = 0;
  refresh_balance_internal();
  refresh_environments_internal();
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
  QString p2p_version, p2p_status;
  CSystemCallWrapper::p2p_version(p2p_version);
  CSystemCallWrapper::p2p_status(p2p_status);
  CRestWorker::Instance()->send_health_request(p2p_version, p2p_status);
  m_report_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::on_balance_updated_sl(CHubBalance balance,
                                      int http_code,
                                      int err_code,
                                      int network_error) {
  UNUSED_ARG(http_code);
  UNUSED_ARG(network_error);

  if (++UPDATED_COMPONENTS_COUNT == 2) {
    m_refresh_timer.start();
  }

  if (err_code == RE_NOT_JSON_DOC) {
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass, lhttp, lerr, lnet);
    if (lerr == RE_SUCCESS) {
      if (++UPDATE_BALANCE_ATTEMPT_COUNT <= UPDATE_ATTEMPT_MAX) {
        CRestWorker::Instance()->update_balance();
      } else {
        UPDATE_BALANCE_ATTEMPT_COUNT = 0;
      }
    } else {
      CApplicationLog::Instance()->LogInfo("Failed to re-login. %d - %d - %d", lhttp, lerr, lnet);
    }
  }
  m_balance = err_code != RE_SUCCESS ? undefined_balance : QString("Balance: %1").arg(balance.value());
  emit balance_updated();
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::on_environments_updated_sl(std::vector<CEnvironment> lst_environments,
                                           int http_code,
                                           int err_code,
                                           int network_error) {
  UNUSED_ARG(http_code);
  CHubController::refresh_environments_res_t rer_res = RER_SUCCESS;

  if (++UPDATED_COMPONENTS_COUNT == 2) {
    m_refresh_timer.start();
  }

  if (err_code == RE_NOT_JSON_DOC) {
    CApplicationLog::Instance()->LogInfo("Failed to refresh environments. Received not json. Trying to re-login");
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass,
                                   lhttp, lerr, lnet);
    if (lerr == RE_SUCCESS) {
      if (++UPDATE_ENVIRONMENTS_ATTEMTP_COUNT <= UPDATE_ATTEMPT_MAX) {
        CRestWorker::Instance()->update_environments();
      } else {
        UPDATE_ENVIRONMENTS_ATTEMTP_COUNT = 0;
      }
    } else {
      CApplicationLog::Instance()->LogInfo("Failed to re-login. %d - %d - %d", lhttp, lerr, lnet);
      return;
    }
  }

  if (err_code || network_error) {
    CApplicationLog::Instance()->LogError("Refresh environments failed. Err_code : %d, Net_err : %d",
                                          err_code, network_error);
    return;
  }

  //that means that about network error notified RestWorker.
  if (err_code && err_code != RE_NETWORK_ERROR) {
    QString err_msg = QString("Refresh environments error : %1").
                      arg(CRestWorker::rest_err_to_str((rest_error_t)err_code));
    CNotificationObserver::Error(err_msg);
    return;
  }

  if (network_error != 0) return;

  if (lst_environments == m_lst_environments_internal) {
    rer_res = m_lst_environments_internal.empty() ? RER_EMPTY : RER_NO_DIFF;
    emit environments_updated(rer_res);
    return;
  }

  {
    SynchroPrimitives::Locker lock(&m_refresh_cs);
    m_lst_environments_internal = std::move(lst_environments);
    m_lst_environments.erase(m_lst_environments.begin(), m_lst_environments.end());
    for (auto env = m_lst_environments_internal.cbegin(); env != m_lst_environments_internal.cend(); ++env) {
      m_lst_environments.push_back(CEnvironment(*env)); //todo check do we need this extra copy?
    }

    m_lst_healthy_environments.erase(m_lst_healthy_environments.begin(), m_lst_healthy_environments.end());
    std::copy_if(m_lst_environments.begin(),
                 m_lst_environments.end(),
                 std::back_inserter(m_lst_healthy_environments),
                 [](const CEnvironment& env){return env.healthy();});
  }
  emit environments_updated(rer_res);
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::start() {
  if (UPDATED_COMPONENTS_COUNT >= 2) {
    m_refresh_timer.start();
  }
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::force_refresh() {
  if (UPDATED_COMPONENTS_COUNT >= 2) {
    refresh_timer_timeout();
  }
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container(const CEnvironment *env,
                                 const CHubContainer *cont,
                                 void* additional_data) {

  ssh_to_container_internal(env, cont, additional_data, ssh_to_cont);
}
////////////////////////////////////////////////////////////////////////////

void
CHubController::ssh_to_container_str(const QString &env_id,
                                     const QString &cont_id,
                                     void *additional_data) {  

  //todo refresh environments
  CEnvironment *env = NULL;
  const CHubContainer *cont = NULL;

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

CHubControllerP2PWorker::CHubControllerP2PWorker(const QString &env_id, 
                                                 const QString &env_hash,
                                                 const QString &env_key,
                                                 const QString &ip,
                                                 const QString &cont_port,
                                                 void *additional_data) :
  m_env_id(env_id),
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
  system_call_wrapper_error_t err = CSystemCallWrapper::join_to_p2p_swarm(m_env_hash,
                                                                          m_env_key,
                                                                          "dhcp");
  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Failed to join to p2p network. Error : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotificationObserver::Error(err_msg);
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

  CNotificationObserver::Info("Checking container. Please, wait");
  static const int MAX_ATTEMTS_COUNT = 25;
  for (int ac = 0; ac < MAX_ATTEMTS_COUNT; ++ac) {
    err = CSystemCallWrapper::check_container_state(m_env_hash,
                                                    m_ip);
    if (err == SCWE_SUCCESS) break;
    QThread::currentThread()->sleep(1);
  }

  if (err != SCWE_SUCCESS) {
    CNotificationObserver::Error("Failed to run SSH because container isn't ready. Try little bit later.");
    emit ssh_to_container_finished((int)SLE_CONT_NOT_READY, m_additional_data);
    return;
  }

  QString key;
  QStringList keys_in_env =
      CSshKeysController::Instance().keys_in_environment(m_env_id);

  if (!keys_in_env.empty()) {
    QString str_file = CSettingsManager::Instance().ssh_keys_storage() +
                       QDir::separator() +
                       keys_in_env[0];
    QFileInfo fi(str_file);

    key = CSettingsManager::Instance().ssh_keys_storage() +
          QDir::separator() + fi.baseName();
  }

  err = CSystemCallWrapper::run_ssh_in_terminal(CSettingsManager::Instance().ssh_user(),
                                                m_ip,
                                                m_cont_port,
                                                key);

  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Run SSH failed. Error code : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotificationObserver::Error(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    emit ssh_to_container_finished((int)SLE_SYSTEM_CALL_FAILED, m_additional_data);
    return ;
  }

  emit ssh_to_container_finished((int)SLE_SUCCESS, m_additional_data);
}
////////////////////////////////////////////////////////////////////////////
