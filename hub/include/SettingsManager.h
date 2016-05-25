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
  static const QString SM_GORJUN_URL;
  static const QString SM_REFRESH_TIME;

  static const QString SM_TERMINAL_PATH;
  static const QString SM_P2P_PATH;

  static const QString SM_NOTIFICATION_DELAY_SEC;
  static const QString SM_SS_UPDATER_PATH;

  static const QString SM_PLUGIN_PORT;
  static const QString SM_SSH_PATH;
  static const QString SM_SSH_USER;
  static const QString SM_SSH_KEYGEN_PATH;

  static const QString SM_RH_USER;
  static const QString SM_RH_PASS;
  static const QString SM_RH_HOST;
  static const QString SM_RH_PORT;

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

  uint16_t m_plugin_port;
  QString m_ssh_path;
  QString m_ssh_user;
  QString m_ssh_keygen_path;

  QString m_rh_host;
  QString m_rh_user;
  QString m_rh_pass;
  QString m_rh_port;

  QString m_gorjun_url;
  QString m_rh_network_interface;

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
  uint16_t plugin_port() const {return m_plugin_port;}
  const QString& ssh_path() const {return m_ssh_path;}
  const QString& ssh_user() const {return m_ssh_user;}
  const QString& ssh_keygen_path() const {return m_ssh_keygen_path;}

  const QString& rh_user() const {return m_rh_user;}
  const QString& rh_pass() const {return m_rh_pass;}
  const QString& rh_host() const {return m_rh_host;}
  const QString& rh_port() const {return m_rh_port;}

  const QString& gorjun_url() const {return m_gorjun_url;}

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

  void set_plugin_port(uint16_t plugin_port) {
    m_plugin_port = plugin_port;
    m_settings.setValue(SM_PLUGIN_PORT, m_plugin_port);
  }

  void set_ssh_path(const QString& ssh_path) {
    m_ssh_path = ssh_path;
    m_settings.setValue(SM_SSH_PATH, m_ssh_path);
  }

  void set_ssh_user(const QString& ssh_user) {
    m_ssh_user = ssh_user;
    m_settings.setValue(SM_SSH_USER, m_ssh_user);
  }

  void set_ssh_keygen_path(const QString& ssh_keygen_path) {
    m_ssh_keygen_path = ssh_keygen_path;
    m_settings.setValue(SM_SSH_KEYGEN_PATH, m_ssh_keygen_path);
  }

  void set_rhip_getter_user(const QString& rhip_getter_user) {
    m_rh_user = rhip_getter_user;
    m_settings.setValue(SM_RH_USER, m_rh_user);
  }

  void set_rhip_getter_pass(const QString& rhip_getter_pass) {
    m_rh_pass = rhip_getter_pass;
    m_settings.setValue(SM_RH_PASS, m_rh_pass);
  }

  void set_rhip_getter_host(const QString& rhip_getter_host) {
    m_rh_host = rhip_getter_host;
    m_settings.setValue(SM_RH_HOST, m_rh_host);
  }

  void set_rhip_getter_port(const QString& rhip_getter_port) {
    m_rh_port = rhip_getter_port;
    m_settings.setValue(SM_RH_PORT, m_rh_port);
  }

  void set_gorjun_url(const QString& gorjun_url) {
    m_gorjun_url = gorjun_url;
    m_settings.setValue(SM_GORJUN_URL, m_gorjun_url);
  }  
};

#endif // CSETTINGSMANAGER_H
