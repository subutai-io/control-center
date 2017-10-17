#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include "HubController.h"

#include "TrayWebSocketServer.h"
#include "SettingsManager.h"
#include "NotificationObserver.h"
#include "ApplicationLog.h"
#include "SystemCallWrapper.h"

CTrayServer::CTrayServer(quint16 port,
                         QObject *parent) :
  QObject(parent),
  m_web_socket_server(new QWebSocketServer("Tray websocket server",
                                           QWebSocketServer::NonSecureMode,
                                           this)),
  m_lst_clients()
{
  if (m_web_socket_server->listen(QHostAddress::Any, port)) {
    connect(m_web_socket_server, &QWebSocketServer::newConnection,
            this, &CTrayServer::on_new_connection);
    connect(&CHubController::Instance(), &CHubController::ssh_to_container_str_finished,
            this, &CTrayServer::ssh_to_container_finished);
  } else {

    QString err_msg = QString("Can't listen websocket on port : %1 Reason : %2").
                      arg(port).arg(m_web_socket_server->errorString());
    CNotificationObserver::Error(err_msg);
    //CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    qFatal("%s", err_msg.toStdString().c_str());
  }
}
////////////////////////////////////////////////////////////////////////////

CTrayServer::~CTrayServer() {
  m_web_socket_server->close();
  qDeleteAll(m_lst_clients.begin(), m_lst_clients.end());
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::handle_current_user(const QString &msg,
                                 QWebSocket *pClient) {
  UNUSED_ARG(msg);
  //CApplicationLog::Instance()->LogTrace("*** handle_current_user ***");
  qInfo("*** handle_current_user ***");
  pClient->sendTextMessage(CHubController::Instance().current_user());
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::handle_ss_ip(const QString &msg,
                               QWebSocket *pClient) {
  UNUSED_ARG(msg);
  std::string rh_ip;
  int ec;
  system_call_wrapper_error_t err =
      CSystemCallWrapper::get_rh_ip_via_libssh2(
        CSettingsManager::Instance().rh_host().toStdString().c_str(),
        CSettingsManager::Instance().rh_port(),
        CSettingsManager::Instance().rh_user().toStdString().c_str(),
        CSettingsManager::Instance().rh_pass().toStdString().c_str(),
        ec,
        rh_ip);

  if (err == SCWE_SUCCESS && !rh_ip.empty()) {
    QString response = QString("code:%1%%%error=%2%%%success==%3")
                       .arg(SCWE_SUCCESS)
                       .arg("")
                       .arg(QString::fromStdString(rh_ip));
    pClient->sendTextMessage(response);
  } else {
    QString response = QString("code:%1%%%error=%2%%%success==%3")
                       .arg(err)
                       .arg(CSystemCallWrapper::scwe_error_to_str(err))
                       .arg("");
    pClient->sendTextMessage(response);
  }
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::handle_ssh(const QString &msg,
                             QWebSocket *pClient) {
  int index_of = msg.indexOf("cmd:ssh");
  QStringList args = msg.mid(index_of - 1 + 7, -1).split("%%%");
  if (args.count() != 3) {
    QString response = QString("code:%1%%%error==%2%%%success==%3")
                       .arg(SLE_LAST_ERR+1)
                       .arg(QString("Wrong command \"%1\"").arg(msg))
                       .arg("");
    pClient->sendTextMessage(response);
    return;
  }  
  CHubController::Instance().ssh_to_container_str(args[1], args[2], (void*)pClient);
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::handle_wrong_command(const QString &msg,
                                       QWebSocket *pClient) {
  QString response = QString("code:%1%%%error==%2%%%success==%3")
                     .arg(SLE_LAST_ERR+1)
                     .arg(QString("Unknown command \"%1\"").arg(msg))
                     .arg("");
  pClient->sendTextMessage(response);
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::on_new_connection() {
  QWebSocket *n_sock = m_web_socket_server->nextPendingConnection();
  connect(n_sock, &QWebSocket::textMessageReceived, this, &CTrayServer::process_text_msg);
  connect(n_sock, &QWebSocket::binaryMessageReceived, this, &CTrayServer::process_bin_msg);
  connect(n_sock, &QWebSocket::disconnected, this, &CTrayServer::socket_disconnected);
  m_lst_clients << n_sock;
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::process_text_msg(QString msg) {
  struct cmd_t {
    const char* start_with;
    void (*pf_handler)(const QString&, QWebSocket*);
  };
  static cmd_t dct_commands[] = {
    {"cmd:current_user", handle_current_user},
    {"cmd:ss_ip", handle_ss_ip},
    {"cmd:ssh", handle_ssh},
    {NULL, handle_wrong_command}
  };

  cmd_t* tc = dct_commands;
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
  if (!pClient)
    return;

  for (;tc->start_with;++tc) {
    if (msg.indexOf(tc->start_with) != -1) break;
  }
  tc->pf_handler(msg, pClient);
}
////////////////////////////////////////////////////////////////////////////

void
CTrayServer::process_bin_msg(QByteArray msg) {
  UNUSED_ARG(msg);
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
