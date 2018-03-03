#ifndef DLGPEER_H
#define DLGPEER_H

#include <QDialog>
#include "HubController.h"
#include "SystemCallWrapper.h"

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
  QString current_peer_name;
  QString current_peer_id;
  QDialog* registration_dialog;

signals:
  void ssh_to_rh_sig(const QString&);
private slots:
  void ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code);
  void registerPeer();
  void unregisterPeer();
  void regDlgClosed();
};

#endif // DLGPEER_H
