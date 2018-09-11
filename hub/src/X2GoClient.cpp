#include "X2GoClient.h"
#include "HubController.h"

QString X2GoClient::x2goclient_config_path() {
  static const QString config_file = "x2goclient_session.ini";
  QString conf_file_path;
  QStringList lst_config=
      QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
  qDebug() << "Finding a path for x2goclient conig files";
  do {
    if (lst_config.empty())
      break;

    QString dir_path = lst_config[0] + QDir::separator() + "subutai";

    QDir dir_config(dir_path);
    if (!dir_config.exists()) {
      if (!dir_config.mkdir(dir_path)) {
        //todo log this
        break;
      }
    }

    QFileInfo fi(dir_path);
    if (!fi.isWritable()) {
      //todo log this
      break;
    }

    conf_file_path = dir_path + QDir::separator() + config_file;
    QFile conf_file(conf_file_path);

    qDebug() << "X2Go session file path: "
             << conf_file_path;

    if (conf_file.exists()) {
      conf_file.remove();
      qDebug() << "Removed old file X2Go session: "
               << conf_file_path;
    }

    return conf_file_path;
  } while (false);

  conf_file_path = QApplication::applicationDirPath() + QDir::separator() + config_file;
  QFile conf_file(conf_file_path);

  qDebug() << "X2GO session file path: "
           << conf_file_path;

  if (conf_file.exists()) {
    conf_file.remove();
    qDebug() << "Removed old file X2Go session: "
             << conf_file_path;
  }

  return conf_file_path;
}

X2GoClient::X2GoClient(QObject *parent) :
  QObject(parent), m_settings(x2goclient_config_path(), QSettings::IniFormat)
{

}

void X2GoClient::add_session(const CHubContainer *cont, QString username, QString key) {
  CSystemCallWrapper::container_ip_and_port cip =
      CSystemCallWrapper::container_ip_from_ifconfig_analog(cont->port(), cont->ip(), cont->rh_ip());
  qDebug()
      << "Container: " << cont->name()
      << "Ip from ifconfig: " << cip.ip
      << "Port from ifconfig: " << cip.port;
  qDebug() << "Adding a session file with following parameters: \n"

           << "Name: " << cont->name() << "\n"
           << "Host: " << cip.ip << "\n"
           << "Port: " << cip.port << "\n"
           << "User: " << username << "\n"
           << "Key: " << key << "\n"
           << "Destop Env: " << cont->desk_env();


  m_settings.beginGroup(cont->id());

  // dynamic parameters
  m_settings.setValue("name", cont->name());
  m_settings.setValue("host", cip.ip);
  m_settings.setValue("sshport", cip.port);
  m_settings.setValue("user", username);
  m_settings.setValue("key", key);

  if(cont->desk_env().isNull() || cont->desk_env().isEmpty() || cont->desk_env() == "\n")
    m_settings.setValue("command", "MATE");
  else
    m_settings.setValue("command", cont->desk_env());

  m_settings.setValue("usesshproxy", false);
  m_settings.setValue("sshproxytype", "SSH");
  m_settings.setValue("sshproxyport", "22");
  m_settings.setValue("sshproxysamepass", false);
  m_settings.setValue("sshproxysameuser", false);
  m_settings.setValue("sshproxyautologin", false);
  m_settings.setValue("sshproxykrblogin", false);

  // static parameters
  m_settings.setValue("autologin", true);
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


