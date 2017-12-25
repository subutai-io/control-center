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
#include "VirtualMachine.h"
#include "DlgNotification.h"

namespace Ui {
  class TrayControlWindow;
}

class CVBPlayerItem : public QWidget {
    Q_OBJECT
private:
    QString m_vm_player_item_id;
    QAction* m_player_item_act;

    QHBoxLayout *p_h_Layout;
    QLabel *m_lbl_name;
    QLabel *m_lbl_state;

    QPushButton *m_btn_play;
    QPushButton *m_btn_stop;
    QPushButton *m_btn_add;

public:
    CVBPlayerItem(const CVirtualMachine* vm, QWidget* parent);
    virtual ~CVBPlayerItem();
    void set_buttons(MachineState_T state);

private slots:
    void vbox_menu_btn_play_released();
    void vbox_menu_btn_stop_released();

signals:
    void vbox_menu_btn_play_released_signal(const QString& vm_id);
    void vbox_menu_btn_stop_released_signal(const QString& vm_id);
};
////////////////////////////////////////////////////////////////////////////

class CVBPlayer : public QWidget{
    Q_OBJECT
private:
    QVBoxLayout *m_vLayout;
    QAction* m_player_act;
    QLabel *m_lblHeader;
    QHBoxLayout *m_horHeaderLayout;
    int m_vm_count;

public:
    CVBPlayer(QWidget *parent);
    virtual ~CVBPlayer();
    void add(CVBPlayerItem* pItem);
    void remove(CVBPlayerItem* pItem);
    int vm_count(void) const {return m_vm_count;}
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class TrayControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TrayControlWindow(QWidget *parent = 0);
  virtual ~TrayControlWindow();
  static TrayControlWindow* Instance(){
    static TrayControlWindow *tcw = new TrayControlWindow();
    return tcw;
  }
  void Init (){;} /* do nothing */
private:
  CVBPlayer *m_w_Player;
  Ui::TrayControlWindow *ui;

  static QDialog *last_generated_env_dlg(QWidget *p);
  void generate_env_dlg(const CEnvironment *env);
  static QDialog *m_last_generated_env_dlg;

  static QDialog *last_generated_peer_dlg(QWidget *p);
  void generate_peer_dlg(CMyPeerInfo *peer, std::pair<QString, QString>);
  static QDialog *m_last_generated_peer_dlg;
  /*hub end*/

  /*vbox*/
  QWidgetAction *m_vboxAction;
  std::map<QString, CVBPlayerItem*>  m_dct_player_menus;
  void add_vm_menu(const QString &vm_id);
  void remove_vm_menu(const QString &vm_id);
  /*vbox end*/

  /*tray icon*/
  QMenu *m_hub_menu;
  QMenu *m_peer_menu;
  QMenu *m_vbox_menu;

  QAction *m_act_ssh_keys_management;
  QAction *m_act_quit;
  QAction *m_act_settings;
  QAction *m_act_balance;
  QAction *m_act_vbox;
  QAction *m_act_hub;
  QAction *m_act_launch;
  QAction *m_act_launch_Hub;
  QAction *m_act_about;
  QAction *m_act_logout;
  QAction *m_act_notifications_history;

  QSystemTrayIcon* m_sys_tray_icon;
  QMenu* m_tray_menu;  

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

  /*virtualbox slots*/
  void show_vbox();
  void launch_Hub();
  void launch_ss();

  /*hub slots*/
  void show_notifications_triggered();

private slots:
  /*tray slots*/
  void dialog_closed(int unused);
  void application_quit();
  void notification_received(CNotificationObserver::notification_level_t level,
                             const QString& msg, DlgNotification::NOTIFICATION_ACTION_TYPE action_type);
  void logout();
  void login_success();
  void launch_ss_console_finished_sl();

  /*virtualbox slots*/
  void fill_vm_menu();
  void vm_added(const QString& vm_id);
  void vm_removed(const QString& vm_id);
  void vm_state_changed(const QString& vm_id);
  void vm_session_state_changed(const QString& vm_id);
  void vmc_player_act_released(const QString& vm_id);
  void vbox_menu_btn_play_triggered(const QString& vm_id);
  void vbox_menu_btn_stop_triggered(const QString& vm_id);
  void vbox_menu_btn_add_triggered(const QString& vm_id);
  void vbox_menu_btn_rem_triggered(const QString& vm_id);

  /*hub slots*/
  void environments_updated_sl(int rr);
  void balance_updated_sl();
  void my_peers_updated_sl();

  void got_ss_console_readiness_sl(bool is_ready, QString err);

  void ssh_to_container_triggered(const CEnvironment *env,
                                      const CHubContainer *cont, void *action);
  void desktop_to_container_triggered(const CEnvironment *env,
                                      const CHubContainer *cont, void *action);

  void ssh_key_generate_triggered();
  void ssh_to_container_finished(int result, void* additional_data);
  void desktop_to_container_finished(int result, void* additional_data);

  /*updater*/
  void update_available(QString file_id);
  void update_finished(QString file_id, bool success);

public slots:
  void tray_icon_is_activated_sl(QSystemTrayIcon::ActivationReason reason);
};
////////////////////////////////////////////////////////////////////////////
#endif // TRAYCONTROLWINDOW_H
