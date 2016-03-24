#ifndef CSETTINGSMANAGER_H
#define CSETTINGSMANAGER_H

#include <stdint.h>
#include <QSettings>
#include <QString>

class CSettingsManager
{
private:
  static const QString SS_UPDATER_APP_NAME;

  static const QString ORG_NAME;
  static const QString APP_NAME;

  static const QString SM_LOGIN;
  static const QString SM_PASSWORD;
  static const QString SM_REMEMBER_ME;

  static const QString SM_POST_URL;
  static const QString SM_GET_URL;
  static const QString SM_REFRESH_TIME;

  static const QString SM_TERMINAL_PATH;
  static const QString SM_P2P_PATH;

  static const QString SM_NOTIFICATION_DELAY_SEC;
  static const QString SM_SS_UPDATER_PATH;

  static const QString SM_UPDATER_USER;
  static const QString SM_UPDATER_PASS;
  static const QString SM_UPDATER_HOST;
  static const QString SM_UPDATER_PORT;

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
  QString m_ss_updater_path;

  uint32_t m_notification_delay_sec;

  //todo remove it from settings after getting it somehow from somewhere
  QString m_updater_host;
  QString m_updater_user;
  QString m_updater_pass;
  QString m_updater_port;

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
  const QString& ss_updater_path() const {return m_ss_updater_path;}
  const QString& updater_user() const{return m_updater_user;}
  const QString& updater_pass() const{return m_updater_pass;}
  const QString& updater_host() const{return m_updater_host;}
  const QString& updater_port() const{return m_updater_port;}

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

  void set_terminal_path(const QString& ssh_term_path) {
    m_terminal_path = ssh_term_path;
    m_settings.setValue(SM_TERMINAL_PATH, m_terminal_path);
  }

  void set_p2p_path(const QString& p2p_path) {
    m_p2p_path = p2p_path;
    m_settings.setValue(SM_P2P_PATH, m_p2p_path);
  }

  void set_notification_delay_sec(uint32_t delay_sec) {
    m_notification_delay_sec = delay_sec;
    if (delay_sec > NOTIFICATION_DELAY_MAX) m_notification_delay_sec = NOTIFICATION_DELAY_MAX;
    if (delay_sec < NOTIFICATION_DELAY_MIN) m_notification_delay_sec = NOTIFICATION_DELAY_MIN;
    m_settings.setValue(SM_NOTIFICATION_DELAY_SEC, m_notification_delay_sec);
  }

  void set_ss_updater_path(const QString& ss_path) {
    m_ss_updater_path = ss_path;
    m_settings.setValue(SM_SS_UPDATER_PATH, m_ss_updater_path);
  }

  void set_updater_user(const QString& updater_user) {
    m_updater_user = updater_user;
    m_settings.setValue(SM_UPDATER_USER, m_updater_user);
  }

  void set_updater_pass(const QString& updater_pass) {
    m_updater_pass = updater_pass;
    m_settings.setValue(SM_UPDATER_PASS, m_updater_pass);
  }

  void set_updater_host(const QString& updater_host) {
    m_updater_host = updater_host;
    m_settings.setValue(SM_UPDATER_HOST, m_updater_host);
  }

  void set_updater_port(const QString& updater_port) {
    m_updater_port = updater_port;
    m_settings.setValue(SM_UPDATER_PORT, m_updater_port);
  }
};

#endif // CSETTINGSMANAGER_H
