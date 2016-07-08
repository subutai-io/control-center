#include <QApplication>
#include <QDir>
#include <QUuid>

#include "SettingsManager.h"
#include "ApplicationLog.h"

const QString CSettingsManager::ORG_NAME("Optimal-dynamics");
const QString CSettingsManager::APP_NAME("SS_Tray");

const QString CSettingsManager::SM_LOGIN("Login");
const QString CSettingsManager::SM_PASSWORD("Password");
const QString CSettingsManager::SM_REMEMBER_ME("Remember_Me");
const QString CSettingsManager::SM_GORJUN_URL("Gorjun_Url");

const QString CSettingsManager::SM_REFRESH_TIME("Refresh_Time_Sec");
const QString CSettingsManager::SM_TERMINAL_PATH("Terminal_Path");
const QString CSettingsManager::SM_P2P_PATH("P2P_Path");

const QString CSettingsManager::SM_NOTIFICATION_DELAY_SEC("Notification_Delay_Sec");

#ifdef RT_OS_LINUX
const QString CSettingsManager::LIBSSH2_APP_NAME("libssh2_app");
#elif RT_OS_WINDOWS
const QString CSettingsManager::LIBSSH2_APP_NAME("libssh2_app.exe");
#elif RT_OS_DARWIN
const QString CSettingsManager::LIBSSH2_APP_NAME("libssh2_app");
#endif

const QString CSettingsManager::SM_PLUGIN_PORT("Plugin_Port");
const QString CSettingsManager::SM_SSH_PATH("Ssh_Path");
const QString CSettingsManager::SM_SSH_USER("Ssh_User");
const QString CSettingsManager::SM_SSH_KEYGEN_PATH("Ssh_Keygen_Path");

const QString CSettingsManager::SM_RH_USER("Rh_User");
const QString CSettingsManager::SM_RH_PASS("Rh_Pass");
const QString CSettingsManager::SM_RH_HOST("Rh_Host");
const QString CSettingsManager::SM_RH_PORT("Rh_Port");

const QString CSettingsManager::SM_LOGS_STORAGE("Rh_Logs_Storage");
const QString CSettingsManager::SM_SSH_KEYS_STORAGE("Rh_Ssh_Keys_Storage");

const QString CSettingsManager::SM_TRAY_GUID("Tray_Guid");

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
  m_health_url("http://158.181.174.23:8080/rest/v1/tray/tray-data"),
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
  m_libssh2_path(QApplication::applicationDirPath() + QDir::separator() + LIBSSH2_APP_NAME),
  m_notification_delay_sec(5),
  m_plugin_port(9998),
  m_ssh_path("ssh"),
  m_ssh_user("root"),
  m_ssh_keygen_path("ssh-keygen"),
  m_rh_host("127.0.0.1"),
  m_rh_user("ubuntu"),
  m_rh_pass("ubuntu"),
  m_rh_port("4567"),
  m_gorjun_url("https://cdn.subut.ai:8338/kurjun/rest/%1"),
  m_logs_storage(QApplication::applicationDirPath()),
  m_ssh_keys_storage(QApplication::applicationDirPath()),
  m_tray_guid("")
{
  QString tmp("");
  str_setting_val dct_str_vals[] = {
    {m_terminal_path, SM_TERMINAL_PATH},
    {m_p2p_path, SM_P2P_PATH},
    {m_ssh_path, SM_SSH_PATH},
    {m_ssh_user, SM_SSH_USER},
    {m_ssh_keygen_path, SM_SSH_KEYGEN_PATH},
    {m_rh_host, SM_RH_HOST},
    {m_rh_pass, SM_RH_PASS},
    {m_rh_port, SM_RH_PORT},
    {m_rh_user, SM_RH_USER},
    {m_gorjun_url, SM_GORJUN_URL},
    {m_logs_storage, SM_LOGS_STORAGE},
    {m_ssh_keys_storage, SM_SSH_KEYS_STORAGE},
    {m_tray_guid, SM_TRAY_GUID},
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

  if (m_tray_guid.isEmpty()) {
    m_tray_guid = QUuid::createUuid().toString();
    m_settings.setValue(SM_TRAY_GUID, m_tray_guid);
  }
}
////////////////////////////////////////////////////////////////////////////

void
CSettingsManager::set_logs_storage(const QString &logs_storage) {
  m_logs_storage = logs_storage;
  m_settings.setValue(SM_LOGS_STORAGE, m_logs_storage);
  CApplicationLog::Instance()->SetDirectory(m_logs_storage.toStdString());
}
////////////////////////////////////////////////////////////////////////////

#define SET_FIELD_DEF(f, fn, t) void CSettingsManager::set_##f(const t f) {m_##f = f; m_settings.setValue(fn, m_##f);}
  SET_FIELD_DEF(login, SM_LOGIN, QString&)
  SET_FIELD_DEF(password, SM_PASSWORD, QString&)
  SET_FIELD_DEF(remember_me, SM_REMEMBER_ME, bool)
  SET_FIELD_DEF(refresh_time_sec, SM_REFRESH_TIME, uint32_t)
  SET_FIELD_DEF(terminal_path, SM_TERMINAL_PATH, QString&)
  SET_FIELD_DEF(p2p_path, SM_P2P_PATH, QString&)
  SET_FIELD_DEF(plugin_port, SM_PLUGIN_PORT, uint16_t)
  SET_FIELD_DEF(ssh_path, SM_SSH_PATH, QString&)
  SET_FIELD_DEF(ssh_user, SM_SSH_USER, QString&)
  SET_FIELD_DEF(ssh_keygen_path, SM_SSH_KEYGEN_PATH, QString&)
  SET_FIELD_DEF(rh_user, SM_RH_USER, QString&)
  SET_FIELD_DEF(rh_pass, SM_RH_PASS, QString&)
  SET_FIELD_DEF(rh_host, SM_RH_HOST, QString&)
  SET_FIELD_DEF(rh_port, SM_RH_PORT, QString&)
  SET_FIELD_DEF(gorjun_url, SM_GORJUN_URL, QString&)
  SET_FIELD_DEF(ssh_keys_storage, SM_SSH_KEYS_STORAGE, QString&)
#undef SET_FIELD_DEF
