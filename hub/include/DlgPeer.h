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
  void addLocalPeer(const QString local_ip);

private:
  Ui::DlgPeer *ui;
};

#endif // DLGPEER_H
