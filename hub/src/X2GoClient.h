#ifndef X2GOCLIENT_H
#define X2GOCLIENT_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include "HubController.h"



class X2GoClient : public QObject
{
  Q_OBJECT
public:
  explicit X2GoClient(QObject *parent = nullptr);
  QSettings m_settings;
  void add_session(const CHubContainer *cont, QString username, QString key);
  static QString x2goclient_config_path();
  static X2GoClient& Instance() {
    static X2GoClient x2goclient;
    return x2goclient;
  }

signals:

public slots:
};

#endif // X2GOCLIENT_H
