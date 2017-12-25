#ifndef DLGPEER_H
#define DLGPEER_H

#include <QDialog>
#include "HubController.h"

namespace Ui {
  class DlgPeer;
}

class DlgPeer : public QDialog
{
  Q_OBJECT

public:
  explicit DlgPeer(QWidget *parent = 0);
  ~DlgPeer();
  void addPeer(CMyPeerInfo &peer);
  void addLocalPeer(const QString id, const QString local_ip);
  void noPeer();
  void noLocalPeer();

private:
  Ui::DlgPeer *ui;
signals:
  void launch_ss_console(const QString&);
};

#endif // DLGPEER_H
