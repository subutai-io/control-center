#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

class EchoClient : public QObject
{
Q_OBJECT
public:
  explicit EchoClient(const QUrl &url, bool debug = false,
                      const QString &env_name = "",
                      const QString &cont_name = "",
                      QObject *parent = nullptr);

signals:
  void closed();

private slots:
  void onConnected();
  void onTextMessageReceived(QString message);

private:
  QWebSocket m_webSocket;
  QUrl m_url;
  bool m_debug;
  QString m_env_name;
  QString m_container_name;
};

#endif // ECHOCLIENT_H
