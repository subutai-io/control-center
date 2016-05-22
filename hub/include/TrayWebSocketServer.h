#ifndef TRAYWEBSOCKETSERVER_H
#define TRAYWEBSOCKETSERVER_H

#include <QObject>
#include <QList>
#include <QByteArray>

class QWebSocketServer;
class QWebSocket;

class CTrayServer : public QObject  {
  Q_OBJECT

private:
  QWebSocketServer *m_web_socket_server;
  QList<QWebSocket*> m_lst_clients;

  explicit CTrayServer(quint16 port, QObject *parent = Q_NULLPTR);
  ~CTrayServer();

private slots:
  void on_new_connection();
  void process_text_msg(QString msg);
  void process_bin_msg(QByteArray msg);
  void socket_disconnected();

  void ssh_to_container_finished(int result, void* additional_data);

public:
  static CTrayServer *Instance(void);
  void Init() const {}
};


#endif // TRAYWEBSOCKETSERVER_H
