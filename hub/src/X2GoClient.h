#ifndef X2GOCLIENT_H
#define X2GOCLIENT_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>


static QString x2goclient_config_path() {
  static const QString config_file = "x2goclient_session.ini";
  QStringList lst_config=
      QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
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
    return dir_path + QDir::separator() + config_file;
  } while (false);

  return QApplication::applicationDirPath() + QDir::separator() + config_file;
}

class X2GoClient : public QObject
{
  Q_OBJECT
public:
  explicit X2GoClient(QObject *parent = nullptr);
  QSettings m_settings;
  void add_session(QString cont_id, QString ip, QString port, QString username);

  static X2GoClient& Instance() {
    static X2GoClient x2goclient;
    return x2goclient;
  }

signals:

public slots:
};

#endif // X2GOCLIENT_H
