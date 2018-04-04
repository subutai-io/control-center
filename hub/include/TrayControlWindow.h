#ifndef TRAYCONTROLWINDOW_H
#define TRAYCONTROLWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QObject>
#include <QTimer>
#include <QMenu>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidgetAction>
#include <QMouseEvent>

#include "RestWorker.h"
#include "NotificationObserver.h"
#include "HubController.h"
#include "DlgNotification.h"
#include "SystemCallWrapper.h"
#include "DlgCreatePeer.h"
#include "RestContainers.h"

#include "P2PController.h"
#include "PeerController.h"

namespace Ui {
  class TrayControlWindow;
}


class TrayControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TrayControlWindow(QWidget *parent = 0);
  virtual ~TrayControlWindow();
  P2PStatus_checker::P2P_STATUS p2p_current_status;
  static TrayControlWindow* Instance(){
    static TrayControlWindow *tcw = new TrayControlWindow();
    return tcw;
  }
  void Init (){;} /* do nothing */

  void set_default_peer_id(const QString &id) {
    m_default_peer_id = id;
  }
  QString default_peer_id() const {
    return m_default_peer_id;
  }
  std::map<QString, CEnvironment> environments_table;
private:
  Ui::TrayControlWindow *ui;

  std::vector<CMyPeerInfo> peers_connected;
  std::vector<CLocalPeer> local_peers_connected;
  std::map<QString, CLocalPeer> machine_peers_table;

  static QDialog *last_generated_env_dlg(QWidget *p);
  void generate_env_dlg(const CEnvironment *env);
  static QDialog *m_last_generated_env_dlg;

  static QDialog *last_generated_transferfile_dlg(QWidget *p);
  void generate_transferfile_dlg();
  static QDialog *m_last_generated_tranferfile_dlg;

  QString m_default_peer_id;

  static QDialog *last_generated_peer_dlg(QWidget *p);
  void generate_peer_dlg(CMyPeerInfo *peer, std::pair<QString, QString>, std::vector<CLocalPeer> lp);
  static QDialog *m_last_generated_peer_dlg;
  /*hub end*/


  /*tray icon*/
  QMenu *m_hub_menu;
  QMenu *m_hub_peer_menu;
  QMenu *m_local_peer_menu;

  QAction *m_act_ssh_keys_management;
  QAction *m_act_quit;
  QAction *m_act_settings;
  QAction *m_act_balance;
  QAction *m_act_hub;
  QAction *m_act_launch;
  QAction *m_act_launch_Hub;
  QAction *m_act_about;
  QAction *m_act_logout;
  QAction *m_act_notifications_history;

  QSystemTrayIcon* m_sys_tray_icon;
  QAction *m_act_create_peer;
  QMenu* m_tray_menu;
  QAction *m_act_p2p_status; // p2p status


  std::map<QString, QDialog*> m_dct_active_dialogs;

  void create_tray_actions();
  void create_tray_icon();

  void get_sys_tray_icon_coordinates_for_dialog(int &src_x, int &src_y,
                                     int &dst_x, int &dst_y, int dlg_w, int dlg_h,
                                     bool use_cursor_position);
  void fill_launch_menu();

  /*tray icon end*/

  void show_dialog(QDialog* (*pf_dlg_create)(QWidget*), const QString &title);
public slots:
  /*tray slots*/
  void show_about();
  void show_settings_dialog();
  void launch_Hub();
  void launch_ss();
  void show_create_dialog();

  /*hub slots*/
  void show_notifications_triggered();

  /*p2p status slots*/
  void launch_p2p();
  void launch_p2p_installation();

private slots:
  /*tray slots*/
  void dialog_closed(int unused);
  void application_quit();
  void notification_received(CNotificationObserver::notification_level_t level,
                             const QString& msg, DlgNotification::NOTIFICATION_ACTION_TYPE action_type);
  void logout();
  void login_success();

  /*hub slots*/
  void environments_updated_sl(int rr);
  void balance_updated_sl();
  void my_peers_updated_sl();
  void got_peer_info_sl(int type,
                        QString name,
                        QString dir,
                        QString output);
  void update_peer_menu();

  /*peer slots*/
  void machine_peers_upd_finished();
  void peer_deleted_sl(const QString&);
  void peer_under_modification_sl(const QString &);


  void got_ss_console_readiness_sl(bool is_ready, QString err);


  void ssh_to_rh_triggered(const QString &peer_fingerprint);
  void ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libssh_exit_code);

  void upload_to_container_triggered(const CEnvironment *env, const CHubContainer *cont);
  void ssh_to_container_triggered(const CEnvironment *env, const CHubContainer *cont);
  void desktop_to_container_triggered(const CEnvironment *env, const CHubContainer *cont);

  void ssh_key_generate_triggered();

  void ssh_to_container_finished(const CEnvironment &env,
                                 const CHubContainer &cont,
                                 int result);

  void desktop_to_container_finished(const CEnvironment &env,
                                     const CHubContainer &cont,
                                     int result);

//  void peer_create_finished(int result);
//  void create_peer_triggered(int kkukala);

  /*updater*/
  void update_available(QString file_id);
  void update_finished(QString file_id, bool success);

  /*p2p slots*/
  void update_p2p_status_sl(P2PStatus_checker::P2P_STATUS status);
public slots:
  void tray_icon_is_activated_sl(QSystemTrayIcon::ActivationReason reason);
};
////////////////////////////////////////////////////////////////////////////
#endif // TRAYCONTROLWINDOW_H
