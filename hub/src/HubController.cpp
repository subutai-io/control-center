#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QThread>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>

#include "HubController.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "SettingsManager.h"
#include "SshKeysController.h"
#include "SystemCallWrapper.h"
#include "TrayWebSocketServer.h"
#include "P2PController.h"
#include "X2GoClient.h"

static const QString undefined_balance(QObject::tr("Undefined balance"));
static volatile int UPDATED_COMPONENTS_COUNT = 2;

static const int UPDATE_ATTEMPT_MAX = 5;
static volatile int UPDATE_BALANCE_ATTEMPT_COUNT = 0;
static volatile int UPDATE_ENVIRONMENTS_ATTEMTP_COUNT = 0;

CHubController::CHubController()
    : m_lst_environments_internal(), m_balance(undefined_balance) {
  connect(&m_refresh_timer, &QTimer::timeout, this,
          &CHubController::refresh_timer_timeout);
  connect(CRestWorker::Instance(), &CRestWorker::on_get_balance_finished, this,
          &CHubController::on_balance_updated_sl);
  connect(CRestWorker::Instance(), &CRestWorker::on_get_environments_finished,
          this, &CHubController::on_environments_updated_sl);
  connect(CRestWorker::Instance(), &CRestWorker::on_get_my_peers_finished, this,
          &CHubController::on_my_peers_updated_sl);
  connect(&m_report_timer, &QTimer::timeout, this,
          &CHubController::report_timer_timeout);

  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec() *
                              1000);
  m_refresh_timer.start();

  m_report_timer.setInterval(60 * 1000);  // minute
  m_report_timer.start();
}

CHubController::~CHubController() {
}

void CHubController::desktop_to_container_internal_helper(int result, void *additional_data, finished_slot_t slot) {
  if (slot == desktop_to_cont)
    emit desktop_to_container_finished(result, additional_data);
  else
    emit desktop_to_container_str_finished(result, additional_data);
}

void CHubController::desktop_to_container_internal(const CEnvironment *env,
                                   const CHubContainer *cont,
                                   void *additional_data, finished_slot_t slot) {

  int container_status = P2PController::Instance().get_container_status(env, cont);
  if (container_status != (int) SDLE_SUCCESS) {
     desktop_to_container_internal_helper(container_status, additional_data, slot);
     return;
  }

  QString key = get_env_key(env->id());

  if (key.isEmpty()) {
    //desktop_to_container_internal_helper((int)SDLE_ENV_KEY_FAIL, additional_data, slot);
    //return;
    // for some cases we don't need to have a key
  }


  CSystemCallWrapper::container_ip_and_port cip =
      CSystemCallWrapper::container_ip_from_ifconfig_analog(cont->port(), cont->ip(), cont->rh_ip());

  static const QString x2go_user = "x2go";
  X2GoClient::Instance().add_session(cont, x2go_user, key);

  system_call_wrapper_error_t err = CSystemCallWrapper::run_x2goclient_session(cont->id());

  qInfo(
      "run_x2goclient_session : user : %s, "
      "ip : %s, port : %s, key : %s"
      "env: %s , cont : %s, ",
      CSettingsManager::Instance().ssh_user().toStdString().c_str(),
      cip.ip.toStdString().c_str(), cip.port.toStdString().c_str(), key.toStdString().c_str(),
      env->name().toStdString().c_str(), cont->name().toStdString().c_str());

  if (err != SCWE_SUCCESS) {
    QString err_msg = tr("Run x2goclient session failed. Error code : %1").arg(CSystemCallWrapper::scwe_error_to_str(err));
    qCritical("%s", err_msg.toStdString().c_str());
    desktop_to_container_internal_helper((int)SDLE_SYSTEM_CALL_FAILED, additional_data, slot);
    return;
  }

  desktop_to_container_internal_helper((int)SDLE_SUCCESS, additional_data, slot);
}



void CHubController::ssh_to_container_internal_helper(int result, void *additional_data, finished_slot_t slot) {
  if (slot == ssh_to_cont) {
    emit ssh_to_container_finished(result, additional_data);
  }
  else {
    emit ssh_to_container_str_finished(result, additional_data);
  }
}



void CHubController::ssh_to_container_internal(const CEnvironment *env,
                                               const CHubContainer *cont,
                                               void *additional_data,
                                               finished_slot_t slot) {
  int container_status = P2PController::Instance().get_container_status(env, cont);
  if (container_status != (int) SDLE_SUCCESS) {
    ssh_to_container_internal_helper(container_status, additional_data, slot);
    return;
  }

  QString key = get_env_key(env->id());

  if (key.isEmpty()) {
    // ssh_to_container_internal_helper((int)SDLE_CONT_NOT_READY, additional_data, slot);
    // return;
     // for some cases we don't need to have a key
  }


  CSystemCallWrapper::container_ip_and_port cip =
      CSystemCallWrapper::container_ip_from_ifconfig_analog(cont->port(), cont->ip(), cont->rh_ip());

  system_call_wrapper_error_t err = CSystemCallWrapper::run_ssh_in_terminal(
      CSettingsManager::Instance().ssh_user(), cip.ip, cip.port, key);

  qInfo(
      "run_ssh_in_terminal : user : %s, "
      "ip : %s, port : %s, key : %s"
      "env: %s , cont : %s, ",
      CSettingsManager::Instance().ssh_user().toStdString().c_str(),
      cip.ip.toStdString().c_str(), cip.port.toStdString().c_str(), key.toStdString().c_str(),
      env->name().toStdString().c_str(), cont->name().toStdString().c_str());

  if (err != SCWE_SUCCESS) {
    QString err_msg = tr("Run SSH failed. Error code : %1").arg(CSystemCallWrapper::scwe_error_to_str(err));
    qCritical("%s", err_msg.toStdString().c_str());
    ssh_to_container_internal_helper((int)SDLE_SYSTEM_CALL_FAILED, additional_data, slot);
    return;
  }

  ssh_to_container_internal_helper((int)SDLE_SUCCESS, additional_data, slot);
}

////////////////////////////////////////////////////////////////////////////

QString CHubController::get_env_key(QString env_id) {
  QString key = "";
  QStringList keys_in_env = CSshKeysController::Instance().keys_in_environment(env_id);
  if (!keys_in_env.empty()) {
    QString str_file = CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() + keys_in_env[0];
    QFileInfo fi(str_file);
    key = CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() + fi.baseName();
  }
  return key;
}

////////////////////////////////////////////////////////////////////////////

void CHubController::refresh_my_peers_internal() {
  CRestWorker::Instance()->update_my_peers();
}

////////////////////////////////////////////////////////////////////////////

void CHubController::refresh_environments_internal() {
  CRestWorker::Instance()->update_environments();
}
////////////////////////////////////////////////////////////////////////////

void CHubController::refresh_balance_internal() {
  CRestWorker::Instance()->update_balance();
}

////////////////////////////////////////////////////////////////////////////

void CHubController::refresh_timer_timeout() {
  m_refresh_timer.stop();
  UPDATED_COMPONENTS_COUNT = 0;
  refresh_balance_internal();
  refresh_environments_internal();
  refresh_my_peers_internal();
}
////////////////////////////////////////////////////////////////////////////

void CHubController::settings_changed() {
  m_refresh_timer.stop();
  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec() *
                              1000);
  m_refresh_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void CHubController::report_timer_timeout() {
  m_report_timer.stop();
  QString p2p_version, p2p_status;
  CSystemCallWrapper::p2p_version(p2p_version);
  CSystemCallWrapper::p2p_status(p2p_status);
  CRestWorker::Instance()->send_health_request(p2p_version, p2p_status);
  m_report_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void CHubController::on_balance_updated_sl(CHubBalance balance, int http_code,
                                           int err_code, int network_error) {
  UNUSED_ARG(http_code);
  UNUSED_ARG(network_error);

  if (++UPDATED_COMPONENTS_COUNT == 2) {
    m_refresh_timer.start();
  }

  if (err_code == RE_NOT_JSON_DOC) {
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass, lhttp, lerr,
                                   lnet);
    if (lerr == RE_SUCCESS) {
      if (++UPDATE_BALANCE_ATTEMPT_COUNT <= UPDATE_ATTEMPT_MAX) {
        CRestWorker::Instance()->update_balance();
      } else {
        UPDATE_BALANCE_ATTEMPT_COUNT = 0;
      }
    } else {
      qInfo("Failed to re-login. %d - %d - %d",
                                           lhttp, lerr, lnet);
    }
  }
  m_balance = err_code != RE_SUCCESS
                  ? undefined_balance
                  : tr("Balance: %1").arg(balance.value());
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
    qInfo(
        "Failed to refresh environments. Received not json. Trying to "
        "re-login");
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass, lhttp, lerr,
                                   lnet);
    if (lerr == RE_SUCCESS) {
      if (++UPDATE_ENVIRONMENTS_ATTEMTP_COUNT <= UPDATE_ATTEMPT_MAX) {
        CRestWorker::Instance()->update_environments();
      } else {
        UPDATE_ENVIRONMENTS_ATTEMTP_COUNT = 0;
      }
    } else {
      qInfo("Failed to re-login. %d - %d - %d",
                                           lhttp, lerr, lnet);
      return;
    }
  }

  if (err_code || network_error) {
    qCritical(
        "Refresh environments failed. Err_code : %d, Net_err : %d", err_code,
        network_error);
    return;
  }

  // that means that about network error notified RestWorker.
  if (err_code && err_code != RE_NETWORK_ERROR) {
    QString err_msg =
        tr("Refresh environments error : %1")
            .arg(CRestWorker::rest_err_to_str((rest_error_t)err_code));
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
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
    m_lst_environments.erase(m_lst_environments.begin(),
                             m_lst_environments.end());

    for (auto env = m_lst_environments_internal.cbegin();
         env != m_lst_environments_internal.cend(); ++env) {
      m_lst_environments.push_back(
          CEnvironment(*env));  // todo check do we need this extra copy?
    }

    m_lst_healthy_environments.erase(m_lst_healthy_environments.begin(),
                                     m_lst_healthy_environments.end());

    std::copy_if(m_lst_environments.begin(), m_lst_environments.end(),
                 std::back_inserter(m_lst_healthy_environments),
                 [](const CEnvironment &env) { return env.healthy(); });
  }
  emit environments_updated(rer_res);
}

void CHubController::on_my_peers_updated_sl(std::vector<CMyPeerInfo> lst_peers,
                                            int http_code, int err_code,
                                            int network_error) {
  UNUSED_ARG(http_code);
  UNUSED_ARG(err_code);
  UNUSED_ARG(network_error);
  m_lst_my_peers = lst_peers;
  emit my_peers_updated();
}
////////////////////////////////////////////////////////////////////////////

void CHubController::logout() {
  m_refresh_timer.stop();
  m_report_timer.stop();
  m_lst_environments.clear();
  m_lst_environments_internal.clear();
  m_lst_healthy_environments.clear();
  m_balance = undefined_balance;
  emit environments_updated(RER_SUCCESS);
  emit balance_updated();
}
////////////////////////////////////////////////////////////////////////////

void CHubController::start() {
  if (UPDATED_COMPONENTS_COUNT >= 2) {
    m_refresh_timer.start();
  }
}
////////////////////////////////////////////////////////////////////////////

void CHubController::force_refresh() {
  if (UPDATED_COMPONENTS_COUNT >= 2) {
    refresh_timer_timeout();
  }
}
////////////////////////////////////////////////////////////////////////////

void CHubController::ssh_to_container(const CEnvironment *env,
                                      const CHubContainer *cont,
                                      void *additional_data) {
  ssh_to_container_internal(env, cont, additional_data, ssh_to_cont);
}
void CHubController::ssh_to_container_str(const QString &env_id,
                                          const QString &cont_id,
                                          void *additional_data) {
  CEnvironment *env = NULL;
  const CHubContainer *cont = NULL;
  std::pair<CEnvironment*, const CHubContainer*> res = find_container_by_id(env_id, cont_id);
  env = res.first;
  cont = res.second;
  ssh_to_container_internal(env, cont, additional_data, ssh_to_cont_str);
}

////////////////////////////////////////////////////////////////////////////

void CHubController::desktop_to_container(const CEnvironment *env,
                          const CHubContainer *cont,
                          void *additional_data) {
  desktop_to_container_internal(env, cont, additional_data, desktop_to_cont);
}

void CHubController::desktop_to_container_str(const QString &env_id,
                                          const QString &cont_id,
                                          void *additional_data) {
  CEnvironment *env = NULL;
  const CHubContainer *cont = NULL;

  std::pair<CEnvironment*, const CHubContainer*> res = find_container_by_id(env_id, cont_id);
  env = res.first;
  cont = res.second;

  desktop_to_container_internal(env, cont, additional_data, desktop_to_cont_str);
}

////////////////////////////////////////////////////////////////////////////

std::pair<CEnvironment*, const CHubContainer*> CHubController::find_container_by_id(const QString &env_id,
                                          const QString &cont_id) {
  CEnvironment *env = NULL;
  const CHubContainer *cont = NULL;
  QEventLoop el;
  connect(this, &CHubController::environments_updated, &el, &QEventLoop::quit);
  refresh_environments_internal();
  el.exec();
  {
    SynchroPrimitives::Locker lock(&m_refresh_cs);
    for (auto i = m_lst_environments.begin(); i != m_lst_environments.end(); ++i) {
      if (i->id() != env_id) continue;
      env = &(*i);
      break;
    }
    if (env == NULL) {
      return std::make_pair(nullptr, nullptr);
    }

    for (auto j = env->containers().begin(); j != env->containers().end(); ++j) {
      if (j->id() != cont_id) continue;
      cont = &(*j);
      break;
    }

    if (cont == NULL) {
      return std::make_pair(nullptr, nullptr);
    }
  }
  return std::make_pair(env, cont);
}
////////////////////////////////////////////////////////////////////////////

const QString &CHubController::ssh_desktop_launch_err_to_str(int err) {
  static QString lst_err_str[SDLE_LAST_ERR] = {"Success",
                                              "Environment not found",
                                              "Container not found",
                                              "Container isn't ready",
                                              "Join to p2p swarm failed",
                                              "System call failed"};
  return lst_err_str[err % SDLE_LAST_ERR];
}
////////////////////////////////////////////////////////////////////////////

void CHubController::launch_balance_page() {
  QString chrome_path = CSettingsManager::Instance().chrome_path();
  if (!CCommons::IsApplicationLaunchable(chrome_path)) {
    QStringList args;
    args << "--new-window";
    args << QString(hub_billing_url()).arg(m_user_id);

    if (!QProcess::startDetached(chrome_path, args)) {
      QString err_msg = tr("Launch hub website with google chrome failed");
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      qCritical("%s", err_msg.toStdString().c_str());
      return;
    }
  } else {
    if (!QDesktopServices::openUrl(
            QUrl(QString(hub_billing_url()).arg(m_user_id)))) {
      QString err_msg =
          tr("Launch hub website with default browser failed");
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      qCritical("%s", err_msg.toStdString().c_str());
    }
  }
}

void CHubController::launch_environment_page(int hub_id) {
  QString chrome_path = CSettingsManager::Instance().chrome_path();
  if (!CCommons::IsApplicationLaunchable(chrome_path)) {
    QStringList args;
    args << "--new-window";
    args << QString(hub_billing_url()).arg(m_user_id) + QString("/environments/%1").arg(hub_id); // https://masterhub.subut.ai/users/%1, https://devhub.subut.ai/users/244/environments/2828

    if (!QProcess::startDetached(chrome_path, args)) {
      QString err_msg = tr("Launch hub website with google chrome failed");
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      qCritical("%s", err_msg.toStdString().c_str());
      return;
    }
  } else {
    if (!QDesktopServices::openUrl(
            QUrl(QString(hub_billing_url()).arg(m_user_id) + QString("/environments/%1").arg(hub_id)))) {
      QString err_msg =
          tr("Launch hub website with default browser failed");
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      qCritical("%s", err_msg.toStdString().c_str());
    }
  }
}
////////////////////////////////////////////////////////////////////////////

