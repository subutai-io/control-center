#include "SettingsManager.h"
#include <QApplication>
#include <QDir>

const QString CSettingsManager::ORG_NAME("Optimal-dynamics");
const QString CSettingsManager::APP_NAME("SS_Tray");

const QString CSettingsManager::SM_LOGIN("Login");
const QString CSettingsManager::SM_PASSWORD("Password");
const QString CSettingsManager::SM_REMEMBER_ME("Remember_Me");

const QString CSettingsManager::SM_POST_URL("Post_Url");
const QString CSettingsManager::SM_GET_URL("Get_Url");

const QString CSettingsManager::SM_REFRESH_TIME("Refresh_Time_Sec");
const QString CSettingsManager::SM_TERMINAL_PATH("Terminal_Path");
const QString CSettingsManager::SM_P2P_PATH("P2P_Path");

const QString CSettingsManager::SM_NOTIFICATION_DELAY_SEC("Notification_Delay_Sec");
const QString CSettingsManager::SM_SS_UPDATER_PATH("Ss_Updater_Path");

#ifdef RT_OS_LINUX
const QString CSettingsManager::SS_UPDATER_APP_NAME("libssh2_app");
#elif RT_OS_WINDOWS
const QString CSettingsManager::SS_UPDATER_APP_NAME("libssh2_app.exe");
#elif RT_OS_DARWIN
const QString CSettingsManager::SS_UPDATER_APP_NAME("libssh2_app");
#endif


const QString CSettingsManager::SM_UPDATER_USER("Updater_User");
const QString CSettingsManager::SM_UPDATER_PASS("Updater_Pass");
const QString CSettingsManager::SM_UPDATER_HOST("Updater_Host");
const QString CSettingsManager::SM_UPDATER_PORT("Updater_Port");

const QString CSettingsManager::SM_PLUGIN_PORT("Plugin_Port");
const QString CSettingsManager::SM_SSH_PATH("Ssh_Path");

static const int def_timeout = 120;
CSettingsManager::CSettingsManager() :
  m_settings(QSettings::NativeFormat, QSettings::UserScope, ORG_NAME, APP_NAME),
  m_login(m_settings.value(SM_LOGIN).toString()),
  m_password(m_settings.value(SM_PASSWORD).toString()),
  m_remember_me(m_settings.value(SM_REMEMBER_ME).toBool()),
  m_post_url("https://hub.subut.ai/rest/v1/tray/%1"),
  m_get_url("https://hub.subut.ai/rest/v1/tray/%1"),
  m_refresh_time_sec(def_timeout),
  #ifdef RT_OS_LINUX
  m_terminal_path("xterm"),
  m_p2p_path("p2p"),
  #elif RT_OS_DARWIN
  m_terminal_path("iterm"),
  m_p2p_path("/Applications/Subutai/p2p"),
  #elif RT_OS_WINDOWS
  m_terminal_path("cmd"),
  m_p2p_path("p2p.exe"),
  #endif
  m_ss_updater_path(QApplication::applicationDirPath() + QDir::separator() + SS_UPDATER_APP_NAME),
  m_notification_delay_sec(5),
  m_updater_host("localhost"),
  m_updater_user("local_user"),
  m_updater_pass("password"),
  m_updater_port("22"),
  m_plugin_port(9998),
  m_ssh_path("ssh")
{
  if (!m_settings.value(SM_POST_URL).isNull())
    m_post_url = m_settings.value(SM_POST_URL).toString();

  if (!m_settings.value(SM_GET_URL).isNull())
    m_get_url = m_settings.value(SM_GET_URL).toString();

  if (!m_settings.value(SM_REFRESH_TIME).isNull()) {
    bool ok = false;
    int timeout = m_settings.value(SM_REFRESH_TIME).toInt(&ok);
    m_refresh_time_sec = ok ? timeout : def_timeout;
  }  

  if (!m_settings.value(SM_TERMINAL_PATH).isNull())
    m_terminal_path = m_settings.value(SM_TERMINAL_PATH).toString();

  if (!m_settings.value(SM_P2P_PATH).isNull())
    m_p2p_path = m_settings.value(SM_P2P_PATH).toString();

  if (!m_settings.value(SM_NOTIFICATION_DELAY_SEC).isNull()) {
    bool parsed = false;
    uint32_t nd = m_settings.value(SM_NOTIFICATION_DELAY_SEC).toUInt(&parsed);
    if (parsed)
      set_notification_delay_sec(nd);
  }

  if (!m_settings.value(SM_SS_UPDATER_PATH).isNull())
    m_ss_updater_path = m_settings.value(SM_SS_UPDATER_PATH).toString();

  if (!m_settings.value(SM_UPDATER_HOST).isNull())
    m_updater_host = m_settings.value(SM_UPDATER_HOST).toString();
  if (!m_settings.value(SM_UPDATER_USER).isNull())
    m_updater_user = m_settings.value(SM_UPDATER_USER).toString();
  if (!m_settings.value(SM_UPDATER_PASS).isNull())
    m_updater_pass = m_settings.value(SM_UPDATER_PASS).toString();
  if (!m_settings.value(SM_UPDATER_PORT).isNull())
    m_updater_port = m_settings.value(SM_UPDATER_PORT).toString();
  if (!m_settings.value(SM_PLUGIN_PORT).isNull())
    m_plugin_port = m_settings.value(SM_PLUGIN_PORT).toUInt();
  if (!m_settings.value(SM_SSH_PATH).isNull())
    m_ssh_path = m_settings.value(SM_SSH_PATH).toString();
}
////////////////////////////////////////////////////////////////////////////
