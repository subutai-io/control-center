#ifndef CSETTINGSMANAGER_H
#define CSETTINGSMANAGER_H

#include <stdint.h>
#include <QSettings>
#include <QString>

class CSettingsManager
{
private:
  static const QString LIBSSH2_APP_NAME;

  static const QString ORG_NAME;
  static const QString APP_NAME;

  static const QString SM_LOGIN;
  static const QString SM_PASSWORD;
  static const QString SM_REMEMBER_ME;

  static const QString SM_POST_URL;
  static const QString SM_GET_URL;
  static const QString SM_GORJUN_URL;
  static const QString SM_REFRESH_TIME;

  static const QString SM_TERMINAL_PATH;
  static const QString SM_P2P_PATH;

  static const QString SM_NOTIFICATION_DELAY_SEC;

  static const QString SM_PLUGIN_PORT;
  static const QString SM_SSH_PATH;
  static const QString SM_SSH_USER;
  static const QString SM_SSH_KEYGEN_PATH;

  static const QString SM_RH_USER;
  static const QString SM_RH_PASS;
  static const QString SM_RH_HOST;
  static const QString SM_RH_PORT;

  static const QString SM_LOGS_STORAGE;
  static const QString SM_SSH_KEYS_STORAGE;

  CSettingsManager();

  QSettings m_settings;
  QString m_login;
  QString m_password;
  bool m_remember_me;

  QString m_post_url;
  QString m_get_url;
  int32_t m_refresh_time_sec;
  QString m_terminal_path;
  QString m_p2p_path;
  QString m_libssh2_path;

  uint32_t m_notification_delay_sec;

  uint16_t m_plugin_port;
  QString m_ssh_path;
  QString m_ssh_user;
  QString m_ssh_keygen_path;

  QString m_rh_host;
  QString m_rh_user;
  QString m_rh_pass;
  QString m_rh_port;

  QString m_gorjun_url;
  QString m_logs_storage;
  QString m_ssh_keys_storage;

public:
  static const int NOTIFICATION_DELAY_MIN = 3;
  static const int NOTIFICATION_DELAY_MAX = 300;

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
  const QString& terminal_path() const {return m_terminal_path;}
  const QString& p2p_path() const {return m_p2p_path;}
  uint32_t notification_delay_sec() const {return m_notification_delay_sec;}
  const QString& libssh2_app_path() const {return m_libssh2_path;}
  uint16_t plugin_port() const {return m_plugin_port;}
  const QString& ssh_path() const {return m_ssh_path;}
  const QString& ssh_user() const {return m_ssh_user;}
  const QString& ssh_keygen_path() const {return m_ssh_keygen_path;}

  const QString& rh_user() const {return m_rh_user;}
  const QString& rh_pass() const {return m_rh_pass;}
  const QString& rh_host() const {return m_rh_host;}
  const QString& rh_port() const {return m_rh_port;}

  const QString& gorjun_url() const {return m_gorjun_url;}
  const QString& logs_storage() const {return m_logs_storage;}
  const QString& ssh_keys_storage() const {return m_ssh_keys_storage;}

  ////////////////////////////////////////////////////////////////////////////

  void set_notification_delay_sec(uint32_t delay_sec) {
    m_notification_delay_sec = delay_sec;
    if (delay_sec > NOTIFICATION_DELAY_MAX) m_notification_delay_sec = NOTIFICATION_DELAY_MAX;
    if (delay_sec < NOTIFICATION_DELAY_MIN) m_notification_delay_sec = NOTIFICATION_DELAY_MIN;
    m_settings.setValue(SM_NOTIFICATION_DELAY_SEC, m_notification_delay_sec);
  }

#define SET_FIELD_DECL(f, t) void set_##f(const t f);
  SET_FIELD_DECL(login, QString&)
  SET_FIELD_DECL(password, QString&)
  SET_FIELD_DECL(remember_me, bool)
  SET_FIELD_DECL(get_url, QString&)
  SET_FIELD_DECL(post_url, QString&)
  SET_FIELD_DECL(refresh_time_sec, uint32_t)
  SET_FIELD_DECL(terminal_path, QString&)
  SET_FIELD_DECL(p2p_path, QString&)
  SET_FIELD_DECL(plugin_port, uint16_t)
  SET_FIELD_DECL(ssh_path, QString&)
  SET_FIELD_DECL(ssh_user, QString&)
  SET_FIELD_DECL(ssh_keygen_path, QString&)
  SET_FIELD_DECL(rh_user, QString&)
  SET_FIELD_DECL(rh_pass, QString&)
  SET_FIELD_DECL(rh_host, QString&)
  SET_FIELD_DECL(rh_port, QString&)
  SET_FIELD_DECL(gorjun_url, QString&)
  SET_FIELD_DECL(logs_storage, QString&)
  SET_FIELD_DECL(ssh_keys_storage, QString&)
#undef SET_FIELD_DECL
};

#endif // CSETTINGSMANAGER_H
