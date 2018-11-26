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
#include "SystemCallWrapper.h"
#include "TrayWebSocketServer.h"
#include "P2PController.h"
#include "X2GoClient.h"
#include "SshKeyController.h"

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
        tr("Refreshing Environments error : %1")
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
  emit user_name_updated();
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
      QString profile = CSettingsManager::Instance().default_chrome_profile();
      QString chrome_path = CSettingsManager::Instance().chrome_path();
      qDebug() << "Trying to launch the browser with url: " << url;

      if (CCommons::IsApplicationLaunchable(chrome_path)) {
        QStringList args;
        args << "--new-window";
        args << url
             << QString("--profile-directory=%1").arg(profile);
        if (!QProcess::startDetached(chrome_path, args)) {
          QString err_msg = tr("Unable to redirect to Subutai Bazaar through a "
                               "browser. Be sure that you have Google Chrome "
                               "browser installed in your system, or you can "
                               "install Google Chrome by going to the menu "
                               "> Components. ");
          CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
          qCritical("%s", err_msg.toStdString().c_str());
        }
      }
      else {
          CNotificationObserver::Error(tr("Cannot open Subutai Bazaar without "
                                          "a Google Chrome browser installed "
                                          "in your system. You can install the "
                                          "Google Chrome browser by going to "
                                          "the menu > Components."),
                                       DlgNotification::N_ABOUT);
        }
  } else if (current_browser == "Firefox") {
    QString profile = CSettingsManager::Instance().default_firefox_profile();
    QString firefox_path = CSettingsManager::Instance().firefox_path();
    qDebug() << "Trying to launch the browser with url: " << url;

    if (CCommons::IsApplicationLaunchable(firefox_path)) {
      QStringList args;
      args << "--new-window" << url
           << QString("-P %1").arg(profile);
      if (!QProcess::startDetached(firefox_path, args)) {
        QString err_msg = tr("Unable to redirect to Subutai Bazaar through a "
                             "browser. Be sure that you have Mozilla Firefox "
                             "browser installed in your system, or you can "
                             "install Mozilla Firefox by going to the menu "
                             "> Components. ");
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        qCritical("%s", err_msg.toStdString().c_str());
      }
    } else {
      CNotificationObserver::Error(tr("Cannot open Subutai Bazaar without "
                                      "a Mozilla Firefox browser installed "
                                      "in your system. You can install the "
                                      "Mozilla Firefox browser by going to "
                                      "the menu > Components."),
                                   DlgNotification::N_ABOUT);
    }
  } else if (current_browser == "Edge") {
    qDebug() << "Trying to launch the browser with url:" << url;
    QString edge_ver;
    system_call_wrapper_error_t res = CSystemCallWrapper::edge_version(edge_ver);
    if (res == SCWE_SUCCESS || edge_ver != "undefined") {
      bool err = false;
      QString cmd = QString("start microsoft-edge:%1").arg(url);
      QStringList lst = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
      if (lst.empty()) {
        qCritical("Unable to find standard TEMP location.");
        err = true;
      }
      QString dir = *lst.begin();
      QString file_name("run-edge.bat");
      QFile script(dir + QDir::separator() + file_name);
      if (script.exists()) {
        script.remove();
      }
      script.open(QIODevice::ReadWrite);
      script.write(cmd.toStdString().c_str());
      script.close();
      if (err || !QProcess::startDetached(dir + QDir::separator() + file_name)) {
        QString err_msg = tr("Unable to redirect to Subutai Bazaar through a "
                             "browser. Be sure that you have Microsoft Edge "
                             "browser installed in your system.");
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        qCritical("%s", err_msg.toStdString().c_str());
      }
    } else {
      CNotificationObserver::Error(tr("Cannot open Subutai Bazaar without "
                                      "a Microsoft Edge browser installed "
                                      "in your system."),
                                   DlgNotification::N_NO_ACTION);
    }
  } else if (current_browser == "Safari") {
    qDebug() << "opening safari with given url:" << url;
    QString safari_ver;
    system_call_wrapper_error_t res = CSystemCallWrapper::safari_version(safari_ver);
    if (res == SCWE_SUCCESS || safari_ver != "undefined") {
      QString cmd("open");
      QStringList args;
      args << "-a" << "safari" << url;
      qDebug() << "launching safari..."
               << "cmd:" << cmd
               << "args:" << args;
      if (!QProcess::startDetached(cmd, args)) {
        QString err_msg = tr("Unable to redirect to Subutai Bazaar through a "
                             "browser. Be sure that you have Safari "
                             "browser installed in your system.");
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        qCritical() << "unable to launch safari";
      }
    }
  } else {
    CNotificationObserver::Error(tr("Cannot open Subutai Bazaar without "
                                    "a Safari browser installed "
                                    "in your system."),
                                 DlgNotification::N_NO_ACTION);
  }
}

void CHubController::launch_balance_page() {
  launch_browser(QString(hub_billing_url()).arg(m_user_id));
}

void CHubController::launch_help_page() {
  launch_browser("https://github.com/subutai-io/control-center/wiki/Subutai-Control-Center-Help");
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
  if (P2PController::Instance().is_cont_in_machine(cont.peer_id())) {
    qDebug()
        << "SSH container: " << cont.name()
        << "Ip container: " << cont.ip();
    return CSystemCallWrapper::run_sshkey_in_terminal(CSettingsManager::Instance().ssh_user(), cont.ip(), QString(""), key);
  }

  CSystemCallWrapper::container_ip_and_port cip =
      CSystemCallWrapper::container_ip_from_ifconfig_analog(cont.port(), cont.ip(), cont.rh_ip());
  qDebug()
      << "SSH container: " << cont.name()
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

  // check if container inside of machine
  if (P2PController::Instance().is_cont_in_machine(cont.peer_id())) {
    container_status = SDLE_SUCCESS;
  }

  if (container_status != SDLE_SUCCESS) {
    return container_status;
  }
  if(key.isEmpty()){
    return SDLE_NO_KEY_DEPLOYED;
  }
  system_call_wrapper_error_t run_in_terminal_status = ssh_to_container_in_terminal(cont, key);
  if (run_in_terminal_status != SCWE_SUCCESS) {
    QString err_msg = tr("Failed to SSH into the container. Check the Internet connection and the state of your environment in Bazaar. "
                         "Error details: %1").arg(CSystemCallWrapper::scwe_error_to_str(run_in_terminal_status));
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
  CEnvironment *env = nullptr;
  const CHubContainer *cont = nullptr;
  std::pair<CEnvironment*, const CHubContainer*> res = find_container_by_id(env_id, cont_id);
  env = res.first;
  cont = res.second;
  if (env == nullptr) {
    emit ssh_to_container_from_hub_finished(*env, *cont, SDLE_ENV_NOT_FOUND, additional_data);
  }
  else
  if (cont == nullptr) {
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

  if (P2PController::Instance().is_cont_in_machine(cont.peer_id())) {
    container_status = SDLE_SUCCESS;
  }

  if (container_status != SDLE_SUCCESS) {
    return container_status;
  }
  system_call_wrapper_error_t
      run_in_terminal_status = desktop_to_container_in_x2go(cont, key);

  if (run_in_terminal_status != SCWE_SUCCESS) {
    QString err_msg = tr("Failed to SSH into the container. Check the Internet connection and the state of your environment in Bazaar. "
                         "Error details: %1").arg(CSystemCallWrapper::scwe_error_to_str(run_in_terminal_status));
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
  CEnvironment *env = nullptr;
  const CHubContainer *cont = nullptr;
  std::pair<CEnvironment*, const CHubContainer*> res = find_container_by_id(env_id, cont_id);
  env = res.first;
  cont = res.second;
  if (env == nullptr) {
    emit desktop_to_container_from_hub_finished(*env, *cont, SDLE_ENV_NOT_FOUND, additional_data);
  }
  else
  if (cont == nullptr) {
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
                                              tr("Unable to start SSH into the container because the environment cannot be found. "
                                                 "Verify the state and setup of your environment in Bazaar."),
                                              tr("Unable to start SSH into the container because the container cannot be found. "
                                                 "Verify that you have an active container set up in your environment in Bazaar."),
                                              tr("Unable to start SSH because the container is not ready. "
                                                 "Try again after the container has been set up completely. "
                                                 "To check the container's state, go to its environment in Bazaar."),
                                              tr("Failed to join the swarm because a P2P network cannot be created. "
                                                 "Check the Internet connection and make sure that P2P Daemon is set up and updated."),
                                              tr("Unable to launch an SSH terminal due to a system call failure. "
                                                 "Try restarting your device before relaunching an SSH terminal."),
                                              tr("Cannot SSH to the environment without a deployed SSH key. "
                                                 "To generate and deploy an SSH key, go to the menu > SSH-keys management."),
                                              tr("Cannot file transfer to the environment without a deployed SSH key. "
                                                "To generate and deploy an SSH key, go to the menu > SSH-keys management.")
                                              };
  return lst_err_str[err %  SDLE_LAST_ERR];
}

const QString CHubController::get_env_key(const QString &env_id) {
  QString key_path = "";
  std::vector<SshKey> keys = SshKeyController::Instance().keys_in_environment(env_id);

  if (keys.size() > 0) {
    QFileInfo fi(keys.at(0).path);
    key_path = CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() + fi.baseName();
  }

  return key_path;
}

std::pair<CEnvironment*, const CHubContainer*> CHubController::find_container_by_id(const QString &env_id,
                                          const QString &cont_id) {
  CEnvironment *env = nullptr;
  const CHubContainer *cont = nullptr;
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
    if (env == nullptr) {
      return std::make_pair(nullptr, nullptr);
    }

    for (auto j = env->containers().begin(); j != env->containers().end(); ++j) {
      if (j->id() != cont_id) continue;
      cont = &(*j);
      break;
    }

    if (cont == nullptr) {
      return std::make_pair(env, nullptr);
    }
  }
  return std::make_pair(env, cont);
}

