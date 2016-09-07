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

  static const QString SM_REFRESH_TIME;

  static const QString SM_P2P_PATH;

  static const QString SM_NOTIFICATION_DELAY_SEC;

  static const QString SM_PLUGIN_PORT;
  static const QString SM_SSH_PATH;
  static const QString SM_SSH_USER;

  static const QString SM_RH_USER;
  static const QString SM_RH_PASS;
  static const QString SM_RH_HOST;
  static const QString SM_RH_PORT;

  static const QString SM_LOGS_STORAGE;
  static const QString SM_SSH_KEYS_STORAGE;

  static const QString SM_TRAY_GUID;

  static const QString SM_P2P_UPDATE_FREQ;
  static const QString SM_RH_UPDATE_FREQ;
  static const QString SM_TRAY_UPDATE_FREQ;
  static const QString SM_P2P_AUTOUPDATE;
  static const QString SM_RH_AUTOUPDATE;
  static const QString SM_TRAY_AUTOUPDATE;

  CSettingsManager();

  QSettings m_settings;
  QString m_login;
  QString m_password;
  bool m_remember_me;

  uint32_t m_refresh_time_sec;
  QString m_p2p_path;

  uint32_t m_notification_delay_sec;

  uint16_t m_plugin_port;
  QString m_ssh_path;
  QString m_ssh_user;

  QString m_rh_host;
  QString m_rh_user;
  QString m_rh_pass;
  quint16 m_rh_port;

  QString m_logs_storage;
  QString m_ssh_keys_storage;
  QString m_tray_guid;

  uint32_t m_p2p_update_freq;
  uint32_t m_rh_update_freq;
  uint32_t m_tray_update_freq;
  bool m_p2p_autoupdate;
  bool m_rh_autoupdate;
  bool m_tray_autoupdate;

public:
  static const int NOTIFICATION_DELAY_MIN = 3;
  static const int NOTIFICATION_DELAY_MAX = 300;

  enum update_freq_t {
    UF_MIN1 = 0,
    UF_MIN5,
    UF_MIN10,
    UF_MIN30,
    UF_HOUR1,
    UF_HOUR3,
    UF_HOUR5,
    UF_DAILY,
    UF_WEEKLY,
    UF_MONTHLY,
    UF_NEVER,
    UF_LAST
  };

  static const QString& update_freq_to_str(update_freq_t fr);
  static uint32_t update_freq_to_sec(update_freq_t fr);

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

  uint32_t refresh_time_sec() const {return m_refresh_time_sec;}
  const QString& p2p_path() const {return m_p2p_path;}
  uint32_t notification_delay_sec() const {return m_notification_delay_sec;}
  uint16_t plugin_port() const {return m_plugin_port;}
  const QString& ssh_path() const {return m_ssh_path;}
  const QString& ssh_user() const {return m_ssh_user;}

  const QString& rh_user() const {return m_rh_user;}
  const QString& rh_pass() const {return m_rh_pass;}
  const QString& rh_host() const {return m_rh_host;}
  quint16 rh_port() const {return m_rh_port;}

  const QString& logs_storage() const {return m_logs_storage;}
  const QString& ssh_keys_storage() const {return m_ssh_keys_storage;}
  const QString& tray_guid() const {return m_tray_guid;}

  update_freq_t p2p_update_freq() const { return (update_freq_t)m_p2p_update_freq; }
  update_freq_t rh_update_freq() const { return (update_freq_t)m_rh_update_freq; }
  update_freq_t tray_update_freq() const { return (update_freq_t)m_tray_update_freq; }
  bool p2p_autoupdate() const { return m_p2p_autoupdate; }
  bool rh_autoupdate() const { return m_rh_autoupdate; }
  bool tray_autoupdate() const { return m_tray_autoupdate; }

  ////////////////////////////////////////////////////////////////////////////

  void set_notification_delay_sec(uint32_t delay_sec) {
    m_notification_delay_sec = delay_sec;
    if (delay_sec > NOTIFICATION_DELAY_MAX) m_notification_delay_sec = NOTIFICATION_DELAY_MAX;
    if (delay_sec < NOTIFICATION_DELAY_MIN) m_notification_delay_sec = NOTIFICATION_DELAY_MIN;
    m_settings.setValue(SM_NOTIFICATION_DELAY_SEC, m_notification_delay_sec);
  }

  void set_p2p_update_freq(int fr);
  void set_rh_update_freq(int fr);
  void set_tray_update_freq(int fr);

#define SET_FIELD_DECL(f, t) void set_##f(const t f);
  SET_FIELD_DECL(login, QString&)
  SET_FIELD_DECL(password, QString&)
  SET_FIELD_DECL(remember_me, bool)
  SET_FIELD_DECL(refresh_time_sec, uint32_t)
  SET_FIELD_DECL(p2p_path, QString&)
  SET_FIELD_DECL(plugin_port, uint16_t)
  SET_FIELD_DECL(ssh_path, QString&)
  SET_FIELD_DECL(ssh_user, QString&)
  SET_FIELD_DECL(rh_user, QString&)
  SET_FIELD_DECL(rh_pass, QString&)
  SET_FIELD_DECL(rh_host, QString&)
  SET_FIELD_DECL(rh_port, quint16)
  SET_FIELD_DECL(logs_storage, QString&)
  SET_FIELD_DECL(ssh_keys_storage, QString&)
  SET_FIELD_DECL(p2p_autoupdate, bool)
  SET_FIELD_DECL(rh_autoupdate, bool)
  SET_FIELD_DECL(tray_autoupdate, bool)
#undef SET_FIELD_DECL
};

#endif // CSETTINGSMANAGER_H
