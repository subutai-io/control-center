#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <stdint.h>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QString>

class CSettingsManager : public QObject {
  Q_OBJECT
 private:
  static const QString ORG_NAME;
  static const QString APP_NAME;

  static const QString SM_LOGIN;
  static const QString SM_PASSWORD;
  static const QString SM_REMEMBER_ME;
  static const QString SM_REFRESH_TIME;
  static const QString SM_P2P_PATH;
  static const QString SM_X2GOCLIENT_PATH;
  static const QString SM_NOTIFICATION_DELAY_SEC;
  static const QString SM_PLUGIN_PORT;
  static const QString SM_SSH_PATH;
  static const QString SM_SSH_USER;
  static const QString SM_LOCALE;

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
  static const QString SM_RHMANAGEMENT_FREQ;
  static const QString SM_P2P_AUTOUPDATE;
  static const QString SM_RH_AUTOUPDATE;
  static const QString SM_TRAY_AUTOUPDATE;
  static const QString SM_RHMANAGEMENT_AUTOUPDATE;

  static const QString SM_RTM_DB_DIR;

  static const QString SM_TERMINAL_CMD;
  static const QString SM_TERMINAL_ARG;
  static const QString SM_DCT_NOTIFICATIONS_IGNORE;
  static const QString SM_NOTIFICATIONS_LEVEL;
  static const QString SM_LOGS_LEVEL;

  static const QString SM_USE_ANIMATIONS;
  static const QString SM_PREFERRED_NOTIFICATIONS_PLACE;
  static const QString SM_SSH_KEYGEN_CMD;
  static const QString SM_PYHOCA_CLI;

  static const QString SM_AUTOSTART;
  static const QString SM_CHROME_PATH;
  static const QString SM_SUBUTAI_CMD;

  static const QString EMPTY_STRING;

  CSettingsManager();

  QSettings m_settings;
  QString m_login;

  QByteArray m_password;
  QString m_password_str;

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
  uint32_t m_rh_management_update_freq;
  bool m_p2p_autoupdate;
  bool m_rh_autoupdate;
  bool m_tray_autoupdate;
  bool m_rh_management_autoupdate;

  QString m_terminal_cmd;
  QString m_x2goclient;
  QString m_terminal_arg;

  std::map <QString, QString> m_rh_hosts;
  std::map <QString, quint16> m_rh_ports;
  std::map <QString, QString> m_rh_users;
  std::map <QString, QString> m_rh_passes;




  QMap<QString, QVariant> m_dct_notification_ignore;
  uint32_t m_notifications_level;
  uint32_t m_logs_level;
  uint32_t m_locale;

  bool m_use_animations;
  uint32_t m_preferred_notifications_place;
  QString m_ssh_keygen_cmd;

  bool m_autostart;
  QString m_chrome_path;
  QString m_subutai_cmd;

  void init_password();

 signals:
  void settings_changed();
  void notifications_ignored_changed();

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

  void save_all() {
    m_settings.sync();
    emit settings_changed();
  }
  void clear_all() {
    m_settings.clear();
    m_settings.sync();
  }
  ////////////////////////////////////////////////////////////////////////////

  const QString& login() const { return m_login; }
  const QString& password() const { return m_password_str; }
  bool remember_me() const { return m_remember_me; }

  uint32_t refresh_time_sec() const { return m_refresh_time_sec; }
  uint32_t locale() const { return m_locale; }
  const QString& p2p_path() const { return m_p2p_path; }
  uint32_t notification_delay_sec() const { return m_notification_delay_sec; }
  uint16_t plugin_port() const { return m_plugin_port; }
  const QString& ssh_path() const { return m_ssh_path; }
  const QString& ssh_user() const { return m_ssh_user; }

  const QString& rh_user(const QString &id);
  const QString& rh_pass(const QString &id);
  const QString& rh_host(const QString &id);
  quint16 rh_port(const QString &id);

  const QString& rh_user() const { return m_rh_user; }
  const QString& rh_pass() const { return m_rh_pass; }
  const QString& rh_host() const { return m_rh_host; }
  quint16 rh_port() const { return m_rh_port; }

  const QString& logs_storage() const { return m_logs_storage; }
  const QString& ssh_keys_storage() const { return m_ssh_keys_storage; }
  const QString& tray_guid() const { return m_tray_guid; }

  update_freq_t p2p_update_freq() const {
    return (update_freq_t)m_p2p_update_freq;
  }
  update_freq_t rh_update_freq() const {
    return (update_freq_t)m_rh_update_freq;
  }
  update_freq_t tray_update_freq() const {
    return (update_freq_t)m_tray_update_freq;
  }
  update_freq_t rh_management_update_freq() const {
    return (update_freq_t)m_rh_management_update_freq;
  }
  bool p2p_autoupdate() const { return m_p2p_autoupdate; }
  bool rh_autoupdate() const { return m_rh_autoupdate; }
  bool tray_autoupdate() const { return m_tray_autoupdate; }
  bool rh_management_autoupdate() const { return m_rh_management_autoupdate; }

  bool is_writable() const { return m_settings.isWritable(); }

  // osascript for macOS . don't ask. don't change :(
  const QString& terminal_cmd() const { return m_terminal_cmd; }
  const QString& x2goclient() const { return m_x2goclient; }
  const QString& terminal_arg() const { return m_terminal_arg; }

  bool use_animations() const { return m_use_animations; }

  uint32_t notifications_level() const { return m_notifications_level; }
  uint32_t logs_level() const { return m_logs_level; }
  uint32_t preferred_notifications_place() const {
    return m_preferred_notifications_place;
  }

  const QString& ssh_keygen_cmd() const { return m_ssh_keygen_cmd; }

  bool autostart() const { return m_autostart; }
  const QString& chrome_path() const { return m_chrome_path; }
  const QString& subutai_cmd() const { return m_subutai_cmd; }
  ////////////////////////////////////////////////////////////////////////////

  void set_notification_delay_sec(uint32_t delay_sec) {
    m_notification_delay_sec = delay_sec;
    if (delay_sec > NOTIFICATION_DELAY_MAX)
      m_notification_delay_sec = NOTIFICATION_DELAY_MAX;
    if (delay_sec < NOTIFICATION_DELAY_MIN)
      m_notification_delay_sec = NOTIFICATION_DELAY_MIN;
    m_settings.setValue(SM_NOTIFICATION_DELAY_SEC, m_notification_delay_sec);
  }
  /**********************/

  void set_p2p_update_freq(int fr);
  void set_rh_update_freq(int fr);
  void set_tray_update_freq(int fr);
  void set_rh_management_freq(int fr);
  void set_locale(int fr);
  void set_p2p_path(QString fr);
  void set_x2goclient_path(QString fr);
  void set_rh_pass(const QString &id, const QString &pass);
  void set_rh_user(const QString &id, const QString &user);
  void set_rh_host(const QString &id, const QString &host);
  void set_rh_port(const QString &id, const qint16 &port);




  /**********************/
  bool is_notification_ignored(const QString& msg) const;
  void ignore_notification(const QString& msg);
  void not_ignore_notification(const QString& msg);
  const QMap<QString, QVariant>& dct_notification_ignored() const {
    return m_dct_notification_ignore;
  }
    /**********************/

#define SET_FIELD_DECL(f, t) void set_##f(const t f);
  SET_FIELD_DECL(login, QString&)
  SET_FIELD_DECL(password, QString&)
  SET_FIELD_DECL(remember_me, bool)
  SET_FIELD_DECL(refresh_time_sec, uint32_t)
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
  SET_FIELD_DECL(rh_management_autoupdate, bool)
  SET_FIELD_DECL(terminal_cmd, QString&)
  SET_FIELD_DECL(terminal_arg, QString&)
  SET_FIELD_DECL(use_animations, bool)
  SET_FIELD_DECL(notifications_level, uint32_t)
  SET_FIELD_DECL(logs_level, uint32_t)
  SET_FIELD_DECL(preferred_notifications_place, uint32_t)
  SET_FIELD_DECL(ssh_keygen_cmd, QString&)
  SET_FIELD_DECL(autostart, bool)
  SET_FIELD_DECL(chrome_path, QString&)
  SET_FIELD_DECL(subutai_cmd, QString&)
#undef SET_FIELD_DECL

};

#endif  // CSETTINGSMANAGER_H
