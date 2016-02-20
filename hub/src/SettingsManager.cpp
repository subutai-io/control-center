#include "SettingsManager.h"

const QString CSettingsManager::ORG_NAME("Optimal-dynamics");
const QString CSettingsManager::APP_NAME("SS_Tray");

const QString CSettingsManager::SM_LOGIN("Login");
const QString CSettingsManager::SM_PASSWORD("Password");
const QString CSettingsManager::SM_REMEMBER_ME("Remember_Me");

const QString CSettingsManager::SM_POST_URL("Post_Url");
const QString CSettingsManager::SM_GET_URL("Get_Url");

const QString CSettingsManager::SM_REFRESH_TIME("Refresh_Time_Sec");

static const int def_timeout = 120;
CSettingsManager::CSettingsManager() :
  m_settings(QSettings::NativeFormat, QSettings::UserScope, ORG_NAME, APP_NAME),
  m_login(m_settings.value(SM_LOGIN).toString()),
  m_password(m_settings.value(SM_PASSWORD).toString()),
  m_remember_me(m_settings.value(SM_REMEMBER_ME).toBool()),
  m_post_url("https://hub.subut.ai/rest/v1/tray/%1"),
  m_get_url("https://hub.subut.ai/rest/v1/tray/%1/%2"),
  m_refresh_time_sec(def_timeout)
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
}
////////////////////////////////////////////////////////////////////////////
