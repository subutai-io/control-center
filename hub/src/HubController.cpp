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

  qDebug() << "Current updated components "
           << UPDATED_COMPONENTS_COUNT;
  qDebug() << QString("Balance Update attempts %1-%2")
              .arg(UPDATE_BALANCE_ATTEMPT_COUNT).arg(UPDATE_ATTEMPT_MAX);


  if (err_code == RE_NOT_JSON_DOC) {
    qCritical(
        "Failed to refresh balance. Received not json. Trying to "
        "re-login");
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass, lhttp, lerr,
                                   lnet);
    if (lerr == RE_SUCCESS) {
      qDebug() << "Updating one more time";

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
  qDebug() << "Current updated components "
           << UPDATED_COMPONENTS_COUNT;
  qDebug() << QString("Environments Update attempts %1-%2")
              .arg(UPDATE_ENVIRONMENTS_ATTEMTP_COUNT).arg(UPDATE_ATTEMPT_MAX);

  // Note, Corner case: If err_code is RE_NOT_JSON_DOC, then UPDATED_COMPONENTS_COUNT will be increments, even though it was already updated :(
  if (err_code == RE_NOT_JSON_DOC) {
    qCritical(
        "Failed to refresh environments. Received not json. Trying to "
        "re-login");
    int lhttp, lerr, lnet;
    CRestWorker::Instance()->login(m_current_user, m_current_pass, lhttp, lerr,
                                   lnet);
    if (lerr == RE_SUCCESS) {
      qDebug() << "Updating one more time";
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
    rer_res = RER_ERROR;
    return;
  }

  // that means that about network error notified RestWorker.
  if (err_code && err_code != RE_NETWORK_ERROR) {
    QString err_msg =
        tr("Refresh environments error : %1")
            .arg(CRestWorker::rest_err_to_str((rest_error_t)err_code));
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    rer_res = RER_ERROR;
    return;
  }

  if (network_error != 0) {
    rer_res = RER_ERROR;
    return;
  }

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
/*
    LAUNCHING BROWSER SECTION
*/
////////////////////////////////////////////////////////////////////////////


void CHubController::launch_browser(const QString &url) {
  QString current_browser = CSettingsManager::Instance().default_browser();
  if(current_browser == "Chrome"){
      QString chrome_path = CSettingsManager::Instance().chrome_path();
      qDebug() << "Trying to launch the browser with url: " << url;

      if (CCommons::IsApplicationLaunchable(chrome_path)) {
        QStringList args;
        args << "--new-window";
        args << url
             << "--profile-directory=Default";
        if (!QProcess::startDetached(chrome_path, args)) {
          QString err_msg = tr("Launch bazaar website with google chrome failed");
          CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
          qCritical("%s", err_msg.toStdString().c_str());
        }
      }
      else {
          CNotificationObserver::Error(tr("Please install Google Chrome first. You can install it from \"About\" section"), DlgNotification::N_ABOUT);
        }
      }
}

void CHubController::launch_balance_page() {
  launch_browser(QString(hub_billing_url()).arg(m_user_id));
}

void CHubController::launch_environment_page(const int hub_id) {
  launch_browser(QString(hub_billing_url()).arg(m_user_id) + QString("/environments/%1").arg(hub_id));
}

void CHubController::launch_peer_page(const int peer_id) {
  launch_browser(QString(hub_site() + "/peers/%1").arg(peer_id));
}


////////////////////////////////////////////////////////////////////////////
/*
    SSH INTO CONTAINER SECTION
*/
////////////////////////////////////////////////////////////////////////////


system_call_wrapper_error_t CHubController::ssh_to_container_in_terminal(const CHubContainer &cont, const QString &key) {
  CSystemCallWrapper::container_ip_and_port cip =
      CSystemCallWrapper::container_ip_from_ifconfig_analog(cont.port(), cont.ip(), cont.rh_ip());
  qDebug()
      << "Container: " << cont.name()
      << "Ip from ifconfig: " << cip.ip
      << "Port from ifconfig: " << cip.port;
  return CSystemCallWrapper::run_sshkey_in_terminal(CSettingsManager::Instance().ssh_user(), cip.ip, cip.port, key);
}

ssh_desktop_launch_error_t CHubController::ssh_to_container_internal(const CEnvironment &env, const CHubContainer &cont, const QString &key) {
  qInfo()
      << "User: " << CSettingsManager::Instance().ssh_user()
      << "RH IP: " << cont.rh_ip()
      << "CONT IP: " << cont.ip()
      << "Port: " << cont.port()
      << "Environment: " << env.name()
      << "Container: " << cont.name()
      << "Key: " << key;

  ssh_desktop_launch_error_t
        container_status = P2PController::Instance().is_ready_sdle(env, cont);
  if (container_status != SDLE_SUCCESS) {
    return container_status;
  }
  if(key.isEmpty()){
    return SDLE_NO_KEY_DEPLOYED;
  }
  system_call_wrapper_error_t run_in_terminal_status = ssh_to_container_in_terminal(cont, key);
  if (run_in_terminal_status != SCWE_SUCCESS) {
    QString err_msg = tr("Run SSH failed. Error code : %1")
                      .arg(CSystemCallWrapper::scwe_error_to_str(run_in_terminal_status));
    qCritical() << err_msg;
    return SDLE_SYSTEM_CALL_FAILED;
  }
  return SDLE_SUCCESS;
}

ssh_desktop_launch_error_t CHubController::ssh_to_container(const CEnvironment &env, const CHubContainer &cont) {
  QString key = get_env_key(env.id());
  return ssh_to_container_internal(env, cont, key);
}

void CHubController::ssh_to_container_from_tray(const CEnvironment &env, const CHubContainer &cont) {
  ssh_desktop_launch_error_t res = ssh_to_container(env, cont);
  emit ssh_to_container_from_tray_finished(env, cont, res);
}

void CHubController::ssh_to_container_from_hub(const QString &env_id, const QString &cont_id, void *additional_data) {
  CEnvironment *env = NULL;
  const CHubContainer *cont = NULL;
  std::pair<CEnvironment*, const CHubContainer*> res = find_container_by_id(env_id, cont_id);
  env = res.first;
  cont = res.second;
  if (env == NULL) {
    emit ssh_to_container_from_hub_finished(*env, *cont, SDLE_ENV_NOT_FOUND, additional_data);
  }
  else
  if (cont == NULL) {
    emit ssh_to_container_from_hub_finished(*env, *cont, SDLE_CONT_NOT_FOUND, additional_data);
  }
  else {
    ssh_desktop_launch_error_t res = ssh_to_container(*env, *cont);
    emit ssh_to_container_from_hub_finished(*env, *cont, res, additional_data);
  }
}

////////////////////////////////////////////////////////////////////////////
/*
    DESKTOP INTO CONTAINER SECTION
*/
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CHubController::desktop_to_container_in_x2go(const CHubContainer &cont,
                                                                         const QString &key) {
  static const QString x2go_user = "x2go";
  qDebug() << "Container: " << cont.name();
  X2GoClient::Instance().add_session(&cont, x2go_user, key);
  return CSystemCallWrapper::run_x2goclient_session(cont.id());
}

ssh_desktop_launch_error_t CHubController::desktop_to_container_internal(const CEnvironment &env, const CHubContainer &cont, const QString &key){

  qInfo()
      << "User: " << CSettingsManager::Instance().ssh_user()
      << "RH IP: " << cont.rh_ip()
      << "CONT IP: " << cont.ip()
      << "Port: " << cont.port()
      << "Environment: " << env.name()
      << "Container: " << cont.name()
      << "Key: " << key;

  ssh_desktop_launch_error_t
      container_status = P2PController::Instance().is_ready_sdle(env, cont);

  if (container_status != SDLE_SUCCESS) {
    return container_status;
  }
  system_call_wrapper_error_t
      run_in_terminal_status = desktop_to_container_in_x2go(cont, key);

  if (run_in_terminal_status != SCWE_SUCCESS) {
    QString err_msg = tr("Run SSH failed. Error code : %1")
                      .arg(CSystemCallWrapper::scwe_error_to_str(run_in_terminal_status));
    qCritical() << err_msg;
    return SDLE_SYSTEM_CALL_FAILED;
  }
  return SDLE_SUCCESS;
}

ssh_desktop_launch_error_t CHubController::desktop_to_container(const CEnvironment &env, const CHubContainer &cont) {
  QString key = get_env_key(env.id());
  return desktop_to_container_internal(env, cont, key);
}

void CHubController::desktop_to_container_from_tray(const CEnvironment &env, const CHubContainer &cont) {
  ssh_desktop_launch_error_t res = desktop_to_container(env, cont);
  emit desktop_to_container_from_tray_finished(env, cont, res);
}

void CHubController::desktop_to_container_from_hub(const QString &env_id, const QString &cont_id, void *additional_data) {
  CEnvironment *env = NULL;
  const CHubContainer *cont = NULL;
  std::pair<CEnvironment*, const CHubContainer*> res = find_container_by_id(env_id, cont_id);
  env = res.first;
  cont = res.second;
  if (env == NULL) {
    emit desktop_to_container_from_hub_finished(*env, *cont, SDLE_ENV_NOT_FOUND, additional_data);
  }
  else
  if (cont == NULL) {
    emit desktop_to_container_from_hub_finished(*env, *cont, SDLE_CONT_NOT_FOUND, additional_data);
  }
  else {
    ssh_desktop_launch_error_t res = desktop_to_container(*env, *cont);
    emit desktop_to_container_from_hub_finished(*env, *cont, res, additional_data);
  }
}

////////////////////////////////////////////////////////////////////////////

/*
    SSH&DESKTOP INTO CONTAINER SECTION
*/
////////////////////////////////////////////////////////////////////////////


const QString &CHubController::ssh_desktop_launch_err_to_str(int err) {
  static QString lst_err_str[SDLE_LAST_ERR] = {tr("Success"),
                                              tr("Environment not found"),
                                              tr("Container not found"),
                                              tr("Container isn't ready"),
                                              tr("Join to p2p swarm failed"),
                                              tr("System call failed"),
                                              tr("No key deployed")};
  return lst_err_str[err %  SDLE_LAST_ERR];
}

const QString CHubController::get_env_key(const QString &env_id) {
  QString key = "";
  QStringList keys_in_env = CSshKeysController::Instance().keys_in_environment(env_id);
  if (!keys_in_env.empty()) {
    QString str_file = CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() + keys_in_env[0];
    QFileInfo fi(str_file);
    key = CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() + fi.baseName();
  }
  return key;
}

std::pair<CEnvironment*, const CHubContainer*> CHubController::find_container_by_id(const QString &env_id,
                                          const QString &cont_id) {
  CEnvironment *env = NULL;
  const CHubContainer *cont = NULL;
  qDebug()
      << "Environment ID: " << env_id
      << "Container ID: " << cont_id;

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
      return std::make_pair(env, nullptr);
    }
  }
  return std::make_pair(env, cont);
}

