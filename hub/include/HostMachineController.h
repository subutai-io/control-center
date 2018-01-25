#ifndef HOSTMACHINECONTROLLER_H
#define HOSTMACHINECONTROLLER_H

#include <QObject>
#include <QTcpSocket>
#include <SystemCallWrapper.h>

class HostMachineController : public QObject
{
  enum HOST_CONNECTION_STATUS {
    HOST_CONNECTION_SUCCESS = 0,
    HOST_CONNETION_TIMEOUT,
    HOST_CONNETION_FAIL,
  };

public:
  HostMachineController();

public:
  static HOST_CONNECTION_STATUS connect_to_host(const QString &ip, quint16 port, int connection_timeout) {
    QTcpSocket socket;
    socket.connectToHost(ip, port, QAbstractSocket::NotOpen);
    if (!socket.waitForConnected(connection_timeout)) {
      return HOST_CONNETION_TIMEOUT;
    }
    else if (socket.state() == QAbstractSocket::ConnectedState) {
      socket.close();
      return HOST_CONNECTION_SUCCESS;
    }
    else
      return HOST_CONNETION_FAIL;
  }

  static system_call_wrapper_error_t connect_to_host_scwe(const QString &ip, quint16 port, int connection_timeout)
  {
    HOST_CONNECTION_STATUS res = connect_to_host(ip, port, connection_timeout);
    if (res == HOST_CONNECTION_SUCCESS) {
      return SCWE_SUCCESS;
    }
    else if (res == HOST_CONNETION_TIMEOUT) {
      return SCWE_TIMEOUT;
    }
    else
      return SCWE_CANT_GET_RH_IP;
  }

};

#endif // HOSTMACHINECONTROLLER_H
