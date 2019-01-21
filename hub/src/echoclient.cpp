#include "echoclient.h"
#include <QtCore/QDebug>
#include "HubController.h"

QT_USE_NAMESPACE

EchoClient::EchoClient(const QUrl &url, bool debug,
                       const QString &env_name,
                       const QString &cont_name, QObject *parent) :
    QObject(parent),
    m_url(url),
    m_debug(debug),
    m_env_name(env_name),
    m_container_name(cont_name)
{
  if (m_debug)
    qDebug() << "\nSSH to container...";

  QObject::connect(&m_webSocket, &QWebSocket::connected, this, &EchoClient::onConnected);
  QObject::connect(&m_webSocket, &QWebSocket::disconnected, [this]() {
    emit this->closed();
  });

  QObject::connect(&m_webSocket, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
      [=](QAbstractSocket::SocketError error) {
    qDebug() << "Error: "
             << m_webSocket.errorString();
    emit this->closed();
  });

  QTimer::singleShot(3000, [this]() {
    emit this->closed();
  });

  m_webSocket.open(QUrl(url));
}

void EchoClient::onConnected()
{
  if (m_debug)
      qDebug() << "\nLaunching SSH to container...\n";
  QObject::connect(&m_webSocket, &QWebSocket::textMessageReceived,
          this, &EchoClient::onTextMessageReceived);
  m_webSocket.sendTextMessage(QStringLiteral("cmd:cc%%%")+
                              QStringLiteral("%1").arg(m_env_name)+
                              QStringLiteral("%%%")+
                              QStringLiteral("%1").arg(m_container_name));
}

void EchoClient::onTextMessageReceived(QString message)
{
  QStringList tmp = message.split("%%%");
  if (m_debug) {
    qDebug() << "Result: \n";

    for (auto i:tmp) {
      qDebug() << i
               << "\n";
    }
  }
  m_webSocket.close();
}
