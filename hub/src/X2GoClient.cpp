#include "X2GoClient.h"


X2GoClient::X2GoClient(QObject *parent) :
  QObject(parent), m_settings(x2goclient_config_path(), QSettings::IniFormat)
{

}
#include "HubController.h"
void X2GoClient::add_session(const CHubContainer *cont, QString username, QString key) {
  m_settings.beginGroup(cont->id());

  // dynamic parameters
  m_settings.setValue("name", cont->name());
  m_settings.setValue("host", cont->rh_ip());
  m_settings.setValue("sshport", cont->port());
  m_settings.setValue("user", username);
  m_settings.setValue("key", key);

  m_settings.setValue("usesshproxy", false);
  m_settings.setValue("sshproxytype", "SSH");
  m_settings.setValue("sshproxyport", "22");
  m_settings.setValue("sshproxysamepass", false);
  m_settings.setValue("sshproxysameuser", false);
  m_settings.setValue("sshproxyautologin", false);
  m_settings.setValue("sshproxykrblogin", false);

  // static parameters
  m_settings.setValue("autologin", true);
  m_settings.setValue("command", "MATE");
  m_settings.setValue("speed", 2);
  m_settings.setValue("quality", 9);
  m_settings.setValue("fstunnel", true);
  m_settings.setValue("useiconv", false);
  m_settings.setValue("fullscreen", false);
  m_settings.setValue("type", "auto");
  m_settings.setValue("pack", "16m-jpeg");
  m_settings.setValue("type", "auto");
  m_settings.setValue("iconvto", "UTF-8");
  m_settings.setValue("iconvto", "UTF-8");
  m_settings.setValue("iconvfrom", "ISO8859-1");
  m_settings.setValue("useiconv", false);
  m_settings.setValue("fullscreen", false);
  m_settings.setValue("multidisp", false);
  m_settings.setValue("display", 1);
  m_settings.setValue("maxdim", false);
  m_settings.setValue("rdpclient", "rdesktop");
  m_settings.setValue("width", 800);
  m_settings.setValue("height", 800);
  m_settings.setValue("dpi", 96);
  m_settings.setValue("setdpi", true);
  m_settings.setValue("xinerama", false);
  m_settings.setValue("clipboard", "both");
  m_settings.setValue("usekbd", true);
  m_settings.setValue("type", "auto");
  m_settings.setValue("sound", true);
  m_settings.setValue("soundsystem", "pulse");
  m_settings.setValue("startsoundsystem", true);
  m_settings.setValue("soundtunnel", true);
  m_settings.setValue("defsndport", true);
  m_settings.setValue("sndport", 4713);
  m_settings.setValue("print", true);
  m_settings.setValue("rd", true);
  m_settings.setValue("krblogin", false);
  m_settings.setValue("krbdelegation", false);
  m_settings.setValue("directrdp", false);
  m_settings.setValue("rootless", false);
  m_settings.setValue("published", false);
  m_settings.setValue("xdmcpserver", "localhost");
  m_settings.endGroup();
}


