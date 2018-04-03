#ifndef DLGPEER_H
#define DLGPEER_H

#include <QDialog>
#include "HubController.h"
#include "SystemCallWrapper.h"
#include "PeerController.h"

namespace Ui {
  class DlgPeer;
}

class DlgPeer : public QDialog
{
  Q_OBJECT
public:
  explicit DlgPeer(QWidget *parent = 0);
  ~DlgPeer();
  void addPeer(CMyPeerInfo *hub_peer, std::pair<QString, QString> local_peer, std::vector<CLocalPeer> lp);
  void addLocalPeer(std::pair<QString, QString> peer);
  void addHubPeer(CMyPeerInfo peer);
  void addMachinePeer(CLocalPeer peer);
  void hideSSH();
  void hidePeer();
  void hideEnvs();
  void stopPeer();
  void startPeer();
  void destroyPeer();
  void update_environments(const std::vector<CMyPeerInfo::env_info> envs);
private:
  Ui::DlgPeer *ui;
  QDialog* registration_dialog;

  // some staf about peer
  //global parameters
  QString peer_name;
  QString peer_fingerprint;
  //parameters for ssh section
  QString ssh_port;
  QString ssh_ip;
  QString ssh_user;
  QString ssh_pass;
  bool ssh_available;
  //advanced parameters
  bool advanced;
  QString peer_disk;
  QString peer_os;
  QString peer_cpu;
  QString peer_ram;
  QString peer_bridge;
  QString peer_status;

signals:
  void ssh_to_rh_sig(const QString&);
private slots:
  void ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code);
  void registerPeer();
  void unregisterPeer();
  void regDlgClosed();
};

#endif // DLGPEER_H
