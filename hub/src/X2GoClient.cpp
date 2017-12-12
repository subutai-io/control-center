#include "X2GoClient.h"


X2GoClient::X2GoClient(QObject *parent) :
  QObject(parent), m_settings(x2goclient_config_path(), QSettings::IniFormat)
{

}

void X2GoClient::add_session(QString cont_id, QString ip, QString port, QString username) {
  m_settings.beginGroup(cont_id);
  m_settings.setValue("host", ip);
  m_settings.setValue("sshport", port);
  m_settings.setValue("user", username);
  m_settings.setValue("name", cont_id);


  // static parameters
  m_settings.setValue("autologin", true);
  m_settings.setValue("command", "MATE");
  m_settings.setValue("speed", 2);
  m_settings.setValue("quality", 9);
  m_settings.setValue("fstunnel", true);
  m_settings.setValue("useiconv", false);
  m_settings.setValue("fullscreen", false);
  m_settings.setValue("type", "auto");


  m_settings.endGroup();
}


