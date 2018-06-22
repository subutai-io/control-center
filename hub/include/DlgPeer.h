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
  explicit DlgPeer(QWidget *parent = 0, QString peer_id = "");
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
  void rh_stop_or_start();
  void rh_start();
  void rh_stop();
  void rh_ssh();
  void rh_destroy();
  void rh_reload();
  void rh_update();
  void rh_register_or_unregister();
  void rh_register();
  void rh_unregister();
  void parse_yml();
  bool check_configs();
  void configs();
  bool change_configs();
  void enabled_peer_buttons(bool state);
  //hub peer functions
  void update_environments(const std::vector<CMyPeerInfo::env_info> &envs);
  //local peer functions;
private:
  Ui::DlgPeer *ui;
  //global parameters
  QString peer_fingerprint;
  //parameters from bazaar
  QString peer_name;
  bool hub_available;
  bool envs_available;
  //parameters for ssh section
  QString ssh_port;
  QString ssh_ip;
  QString ssh_user;
  QString ssh_pass;
  bool ssh_available;
  //advanced parameters
  bool advanced; // true if CC found this peer on your machine
  bool management_ua; //management update available
  QString rh_dir;
  QString rh_disk;
  QString rh_os;
  QString rh_cpu;
  QString rh_ram;
  QString rh_bridge;
  QString rh_status;
  QString rh_name;
  QTimer *timer_refresh_machine_peer;
  QDialog *registration_dialog;

signals:
  void ssh_to_rh_sig(const QString&);
  void peer_deleted(const QString&);
  void peer_modified(const QString&);
  void peer_stopped(const QString&);
  void peer_update_peeros(const QString);
private slots:
  void ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code);
  void regDlgClosed();
};

#endif // DLGPEER_H
