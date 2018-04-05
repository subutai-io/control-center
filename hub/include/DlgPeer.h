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
  //ui
  void hideSSH();
  void hidePeer();
  void hideEnvs();
  //peer management functions
  void stopPeer();
  void startPeer();
  void destroyPeer();
  void reloadPeer();
  void parse_yml();
  bool check_configs();
  void configs();
  bool change_configs();
  void enabled_peer_buttons(bool state);
  //hub peer functions
  void update_environments(const std::vector<CMyPeerInfo::env_info> envs);
  //local peer functions;
private:
  Ui::DlgPeer *ui;
  QDialog* registration_dialog;

  // some staf about peer
  //global parameters
  QString peer_name;
  QString peer_fingerprint;
  //parameters for hub
  bool hub_available;
  //parameters for ssh section
  QString ssh_port;
  QString ssh_ip;
  QString ssh_user;
  QString ssh_pass;
  bool ssh_available;
  //advanced parameters
  bool advanced;
  QString peer_dir;
  QString peer_disk;
  QString peer_os;
  QString peer_cpu;
  QString peer_ram;
  QString peer_bridge;
  QString peer_status;

signals:
  void ssh_to_rh_sig(const QString&);
  void peer_deleted(const QString&);
  void peer_modified(const QString&);
  void peer_stopped(const QString&);
private slots:
  void ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code);
  void registerPeer();
  void unregisterPeer();
  void regDlgClosed();
};

#endif // DLGPEER_H
