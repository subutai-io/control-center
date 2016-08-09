#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include "HubController.h"

#include "TrayWebSocketServer.h"
#include "SettingsManager.h"
#include "NotifiactionObserver.h"
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
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
  if (!pClient)
    return;
  if (msg == "cmd:current_user") {
    pClient->sendTextMessage(CHubController::Instance().current_user());
  }
  else if (msg == "cmd:ss_ip") {
    std::string rh_ip;
    int ec;
    system_call_wrapper_error_t err =
        CSystemCallWrapper::get_rh_ip_via_libssh2(
          CSettingsManager::Instance().rh_host().toStdString().c_str(),
          CSettingsManager::Instance().rh_port().toStdString().c_str(),
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
    return;
  } else if (int index_of = msg.indexOf("cmd:ssh") != -1) {
    // 7 is len of cmd::ssh
    QStringList args = msg.mid(index_of - 1 + 7, -1).split("%%%");
    if (args.count() != 3) {
      QString response = QString("code:%1%%%error==%2%%%success==%3")
                         .arg(SLE_LAST_ERR+1)
                         .arg(QString("Wrong command \"%1\"").arg(msg))
                         .arg("");
      pClient->sendTextMessage(response);
      return;
    }
    CHubController::Instance().refresh_environments();
    CHubController::Instance().refresh_containers();
    CApplicationLog::Instance()->LogTrace("process_text_msg: ssh_to_container_str call");
    CHubController::Instance().ssh_to_container_str(args[1], args[2], (void*)pClient);
  } else {
    QString response = QString("code:%1%%%error==%2%%%success==%3")
                       .arg(SLE_LAST_ERR+1)
                       .arg(QString("Unknown command \"%1\"").arg(msg))
                       .arg("");
    pClient->sendTextMessage(response);
    return;
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
  CApplicationLog::Instance()->LogTrace("ssh_to_container_finished : %d", result);
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
