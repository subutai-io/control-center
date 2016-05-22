#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include "HubController.h"

#include "TrayWebSocketServer.h"
#include "SettingsManager.h"
#include "NotifiactionObserver.h"
#include "ApplicationLog.h"

CTrayServer::CTrayServer(quint16 port,
                         QObject *parent) :
  QObject(parent),
  m_web_socket_server(new QWebSocketServer("Tray websocket server",
                                           QWebSocketServer::NonSecureMode,
                                           this)),
  m_lst_clients()
{
  if (m_web_socket_server->listen(QHostAddress::Any, port)) {
    connect(m_web_socket_server, SIGNAL(newConnection()), this, SLOT(on_new_connection()));
    connect(&CHubController::Instance(), SIGNAL(ssh_to_container_str_finished(int,void*)),
            this, SLOT(ssh_to_container_finished(int,void*)));
  } else {
    QString err_msg = QString("Can't listen websocket on port : %1").arg(port);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
  }
}
////////////////////////////////////////////////////////////////////////////

CTrayServer::~CTrayServer() {
  m_web_socket_server->close();
  qDeleteAll(m_lst_clients.begin(), m_lst_clients.end());
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::on_new_connection() {
  QWebSocket *n_sock = m_web_socket_server->nextPendingConnection();
  connect(n_sock, SIGNAL(textMessageReceived(QString)), this, SLOT(process_text_msg(QString)));
  connect(n_sock, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(process_bin_msg(QByteArray)));
  connect(n_sock, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
  m_lst_clients << n_sock;
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::process_text_msg(QString msg) {
  CApplicationLog::Instance()->LogTrace("CTrayServer::process_text_msg : %s", msg.toStdString().c_str());
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
  if (!pClient)
    return;
  if (msg == "cmd:current_user") {
    CApplicationLog::Instance()->LogTrace("1 responce : %s", CHubController::Instance().current_user().toStdString().c_str());
    pClient->sendTextMessage(CHubController::Instance().current_user());
  } else if (int index_of = msg.indexOf("cmd:ssh") != -1) {
    // 7 is len of cmd::ssh
    QStringList args = msg.mid(index_of - 1 + 7, -1).split("%%%");
    CApplicationLog::Instance()->LogTrace("process_text_msg, args created");
    if (args.count() != 3) {
      QString responce = QString("code:%1%%%error==%2%%%success==%3")
                         .arg(SLE_LAST_ERR+1)
                         .arg(QString("Wrong command \"%1\"").arg(msg))
                         .arg("");
      CApplicationLog::Instance()->LogTrace("2 responce : %s", responce.toStdString().c_str());
      pClient->sendTextMessage(responce);
      return;
    }
    CApplicationLog::Instance()->LogTrace("process_text_msg: refresh environments");
    CHubController::Instance().refresh_environments();
    CApplicationLog::Instance()->LogTrace("process_text_msg: refresh containers");
    CHubController::Instance().refresh_containers();

    CApplicationLog::Instance()->LogTrace("process_text_msg: ssh_to_container_str call");

    //todo change this
    CHubController::Instance().ssh_to_container_str(args[1], args[2], (void*)pClient);

  } else {
    //todo unknown command responce
  }
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::process_bin_msg(QByteArray msg) {
  (void)msg; //unused arg;
  //todo implement
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::socket_disconnected() {
  QWebSocket *pClient = qobject_cast<QWebSocket*>(sender());
  if (pClient) {
    m_lst_clients.removeAll(pClient);
    pClient->deleteLater();
  }
}

void
CTrayServer::ssh_to_container_finished(int result,
                                       void *additional_data) {
  QString responce = QString("code:%1%%%error==%2%%%success==%3")
                     .arg(result)
                     .arg(result==SLE_SUCCESS ? "" : CHubController::ssh_launch_err_to_str(result))
                     .arg(result==SLE_SUCCESS ? CHubController::ssh_launch_err_to_str(result) : "");
  QWebSocket *pClient = static_cast<QWebSocket*>(additional_data);
  if (!pClient) return;
  pClient->sendTextMessage(responce);
}
////////////////////////////////////////////////////////////////////////////

CTrayServer*
CTrayServer::Instance() {
  static CTrayServer instance(CSettingsManager::Instance().plugin_port());
  return &instance;
}
////////////////////////////////////////////////////////////////////////////
