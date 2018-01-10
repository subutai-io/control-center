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
  void addPeer(CMyPeerInfo *hub_peer, std::pair<QString, QString> local_peer);
  void addLocalPeer(const QString local_ip);
  void addHubPeer(CMyPeerInfo peer);
private:
  Ui::DlgPeer *ui;
signals:
  void ssh_to_rh_sig(const QString&, void*);
};

#endif // DLGPEER_H
