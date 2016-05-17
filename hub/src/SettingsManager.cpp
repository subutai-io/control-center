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
const QString CSettingsManager::SM_GORJUN_URL("Gorjun_Url");

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
const QString CSettingsManager::SM_SSH_USER("Ssh_User");
const QString CSettingsManager::SM_SSH_KEYGEN_PATH("Ssh_Keygen_Path");

const QString CSettingsManager::SM_RHIP_GETTER_USER("Rhip_Getter_User");
const QString CSettingsManager::SM_RHIP_GETTER_PASS("Rhip_GetterPass");
const QString CSettingsManager::SM_RHIP_GETTER_HOST("Rhip_GetterHost");
const QString CSettingsManager::SM_RHIP_GETTER_PORT("Rhip_GetterPort");
const QString CSettingsManager::SM_RH_NETWORK_INTERFACE("Rh_NetworkInterface");

struct str_setting_val {
  QString& field;
  QString val;
};
////////////////////////////////////////////////////////////////////////////

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
  m_updater_host("127.0.0.1"),
  m_updater_user("local_user"),
  m_updater_pass("password"),
  m_updater_port("22"),
  m_plugin_port(9998),
  m_ssh_path("ssh"),
  m_ssh_user("root"),
  m_ssh_keygen_path("ssh-keygen"),
  m_rhip_getter_host("127.0.0.1"),
  m_rhip_getter_user("ubuntu"),
  m_rhip_getter_pass("ubuntu"),
  m_rhip_getter_port("4567"),
  m_gorjun_url("https://cdn.subut.ai:8338/kurjun/rest/%1"),
#ifndef RT_OS_WINDOWS
  m_rh_network_interface("eth1")
#else
  m_rh_network_interface("wan")
#endif
{
  QString tmp("");
  str_setting_val dct_str_vals[] = {
    {m_post_url, SM_POST_URL},
    {m_get_url, SM_GET_URL},
    {m_terminal_path, SM_TERMINAL_PATH},
    {m_p2p_path, SM_P2P_PATH},
    {m_ss_updater_path, SM_SS_UPDATER_PATH},
    {m_updater_host, SM_UPDATER_HOST},
    {m_updater_user, SM_UPDATER_USER},
    {m_updater_pass, SM_UPDATER_PASS},
    {m_updater_port, SM_UPDATER_PORT},
    {m_ssh_path, SM_SSH_PATH},
    {m_ssh_user, SM_SSH_USER},
    {m_ssh_keygen_path, SM_SSH_KEYGEN_PATH},
    {m_rhip_getter_host, SM_RHIP_GETTER_HOST},
    {m_rhip_getter_pass, SM_RHIP_GETTER_PASS},
    {m_rhip_getter_port, SM_RHIP_GETTER_PORT},
    {m_rhip_getter_user, SM_RHIP_GETTER_USER},
    {m_gorjun_url, SM_GORJUN_URL},
    {m_rh_network_interface, SM_RH_NETWORK_INTERFACE},
    {tmp, ""}
  };

  for (int i = 0; dct_str_vals[i].val != "" ; ++i) {
    if (!m_settings.value(dct_str_vals[i].val).isNull()) {
      dct_str_vals[i].field = m_settings.value(dct_str_vals[i].val).toString();
    }
  }

  if (!m_settings.value(SM_REFRESH_TIME).isNull()) {
    bool ok = false;
    int timeout = m_settings.value(SM_REFRESH_TIME).toInt(&ok);
    m_refresh_time_sec = ok ? timeout : def_timeout;
  }    

  if (!m_settings.value(SM_NOTIFICATION_DELAY_SEC).isNull()) {
    bool parsed = false;
    uint32_t nd = m_settings.value(SM_NOTIFICATION_DELAY_SEC).toUInt(&parsed);
    if (parsed)
      set_notification_delay_sec(nd);
  }  
}
////////////////////////////////////////////////////////////////////////////
