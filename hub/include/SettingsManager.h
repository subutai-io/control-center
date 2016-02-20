#ifndef CSETTINGSMANAGER_H
#define CSETTINGSMANAGER_H

#include <QSettings>
#include <QString>
#include <stdint.h>

class CSettingsManager
{
private:
  static const QString ORG_NAME;
  static const QString APP_NAME;

  static const QString SM_LOGIN;
  static const QString SM_PASSWORD;
  static const QString SM_REMEMBER_ME;

  static const QString SM_POST_URL;
  static const QString SM_GET_URL;
  static const QString SM_REFRESH_TIME;

  CSettingsManager();

  QSettings m_settings;
  QString m_login;
  QString m_password;
  bool m_remember_me;

  QString m_post_url;
  QString m_get_url;
  int32_t m_refresh_time_sec;

public:
  static CSettingsManager& Instance(void) {
    static CSettingsManager instance;
    return instance;
  }

  void save_all() { m_settings.sync(); }
  void clear_all() { m_settings.clear(); m_settings.sync(); }
  ////////////////////////////////////////////////////////////////////////////

  const QString& login() const {return m_login;}
  const QString& password() const {return m_password;}
  bool remember_me() const {return m_remember_me;}
  const QString& get_url() const {return m_get_url;}
  const QString& post_url() const {return m_post_url;}
  int32_t refresh_time_sec() const {return m_refresh_time_sec;}

  void set_login(const QString& login) {
    m_login = login;
    m_settings.setValue(SM_LOGIN, m_login);
  }

  void set_password(const QString& password) {
    m_password = password;
    m_settings.setValue(SM_PASSWORD, m_password);
  }

  void set_remember_me(bool remember) {
    m_remember_me = remember;
    m_settings.setValue(SM_REMEMBER_ME, m_remember_me);
  }

  void set_get_url(const QString& url) {
    m_get_url = url;
    m_settings.setValue(SM_GET_URL, m_get_url);
  }

  void set_post_url(const QString& url) {
    m_post_url = url;
    m_settings.setValue(SM_POST_URL, m_post_url);
  }

  void set_refresh_time_sec(int32_t refresh_time_sec) {
    m_refresh_time_sec = refresh_time_sec;
    m_settings.setValue(SM_REFRESH_TIME, m_refresh_time_sec);
  }
};

#endif // CSETTINGSMANAGER_H
