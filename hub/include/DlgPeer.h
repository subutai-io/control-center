/****************************************************************************
** peer (group of RH connected to one RH with management container)
** CC can control RH found on your machine
** CC can view data about Peer registered to the bazaar
** CC can ssh to the RH found on LAN)
****************************************************************************/
#ifndef DLGPEER_H
#define DLGPEER_H

#include <QDialog>
#include <QLabel>
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
  struct env_label_info
  {
    QLabel *m_env_name;
    QLabel *m_env_user;
    QLabel *m_env_status;
    bool is_deleted;
    env_label_info(){}
    void init(QLabel *env_name,
         QLabel *env_user,
         QLabel *env_status) {
      m_env_name = env_name;
      m_env_user = env_user;
      m_env_status = env_status;
      is_deleted = false; // after each update make false and find removed envs
    }
    ~env_label_info(){}
  };
  explicit DlgPeer(QWidget *parent = 0, QString peer_id = "");
  ~DlgPeer();
  void addPeer(CMyPeerInfo *hub_peer, std::pair<QString, QString> local_peer, std::vector<CLocalPeer> lp); // take all data about peer
  void addLocalPeer(std::pair<QString, QString> peer); // take lan peer data, you need only fingeprint and ip
  void addHubPeer(CMyPeerInfo peer); // take bazaar peer data, when peer is registered to your bazaar account
  void addMachinePeer(CLocalPeer peer); // take machine peer data, which is located on local machine.
  void updatePeer(); // update peer data
  void updateUI(); // change ui according to the peer data
  //ui
  void hideSSH();
  void hidePeer();
  void hideEnvs();
  //peer management functions
  void rh_start();
  void rh_stop();
  void rh_ssh();
  void rh_register();
  void rh_unregister();
  void parse_yml();
  bool check_configs();
  void configs();
  bool change_configs();
  void enabled_peer_buttons(bool state);
  //hub peer functions
  void update_environments(const std::vector<CMyPeerInfo::env_info> &envs);
  // interaction with tray menu
  QString get_peer_name() {
    if (hub_available) return peer_name;
    else return rh_name;
  }
private:
  Ui::DlgPeer *ui;
  //global parameters
  QString peer_fingerprint;
  //parameters from bazaar
  QString peer_name;
  QString peer_id;
  bool hub_available;
  bool envs_available;
  std::map <int, env_label_info> env_info_table;
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
  int rh_provision_step;
  QTimer *refresh_timer;
  QDialog *registration_dialog;

signals:
  void ssh_to_rh_sig(const QString&);
  void peer_deleted(const QString&);
  void peer_modified(const QString&);
  void peer_stopped(const QString&);
  void peer_update_peeros(const QString);
private slots:
  //rh command slots
  void regDlgClosed();
  void rh_stop_or_start_sl();
  void rh_ssh_sl();
  void rh_destroy_sl();
  void rh_reload_sl();
  void rh_update_sl();
  void rh_register_or_unregister_sl();
  //sh command slots
  void ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code);
  //other slots
  void launch_console_sl();
  void launch_bazaar_sl();
};

class HostChecker : public QObject {
    Q_OBJECT
public:
    HostChecker(QObject *parent = nullptr) : QObject (parent) {}
    void init(const QString& m_host);
    void startWork();
    void silenChecker();

private:
    QString m_host;

signals:
    void outputReceived(bool success);
};
#endif // DLGPEER_H
