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

#include "RestWorker.h"
#include "NotificationObserver.h"
#include "HubController.h"
#include "VirtualMachine.h"

namespace Ui {
  class TrayControlWindow;
}

/*!
 * \brief Class for managing VM widget. Allows user to start/pause/stop VM
 */
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

/*!
 * \brief Class for managing CVBPlayerItem list
 */
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

/*!
 * \brief Represents menu item with information about container and environment.
 */
class CHubEnvironmentMenuItem : public QObject {
 Q_OBJECT
private:
  const CEnvironment* m_hub_environment;
  QSystemTrayIcon* m_tray_icon;
  const CHubContainer* m_hub_container;

public:
  CHubEnvironmentMenuItem(const CEnvironment* env,
                          const CHubContainer* cont,
                          QSystemTrayIcon* tray_icon) :
    m_hub_environment(env), m_tray_icon(tray_icon), m_hub_container(cont) {}

  virtual ~CHubEnvironmentMenuItem(){}

signals:
  void action_triggered(const CEnvironment*, const CHubContainer*, void* action);

public slots:
  void internal_action_triggered();

};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Class for main tray window (hidden) representation.
 * Main window of tray application hidden. So user has only tray menu.
 */
class TrayControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TrayControlWindow(QWidget *parent = 0);
  virtual ~TrayControlWindow();

private:
  CVBPlayer *m_w_Player;
  Ui::TrayControlWindow *ui;
  /*hub*/
  std::vector<CHubEnvironmentMenuItem*> m_lst_hub_menu_items;
  /*hub end*/

  /*vbox*/
  QWidgetAction *m_vboxAction;
  std::map<QString, CVBPlayerItem*>  m_dct_player_menus;
  void add_vm_menu(const QString &vm_id);
  void remove_vm_menu(const QString &vm_id);
  /*vbox end*/

  /*tray icon*/
  QMenu *m_hub_menu;
  QMenu *m_vbox_menu;
  QMenu *m_launch_menu;

  QAction *m_act_ssh_keys_management;
  QAction *m_act_quit;
  QAction *m_act_settings;
  QAction *m_act_info;
  QAction *m_act_vbox;
  QAction *m_act_hub;
  QAction *m_act_launch;
  QAction *m_act_launch_SS;
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

  void launch_ss();
  void show_dialog(QDialog* (*pf_dlg_create)(QWidget*), const QString &title);

private slots:
  /*tray slots*/
  void dialog_closed(int unused);
  void show_about();
  void application_quit();
  void show_settings_dialog();  \
  void notification_received(CNotificationObserver::notification_level_t level,
                             const QString& msg);
  void logout();
  void login_success();
  void launch_ss_console_finished_sl();

  /*virtualbox slots*/
  void fill_vm_menu();
  void show_vbox();
  void vm_added(const QString& vm_id);
  void vm_removed(const QString& vm_id);
  void vm_state_changed(const QString& vm_id);
  void vm_session_state_changed(const QString& vm_id);
  void vmc_player_act_released(const QString& vm_id);
  void vbox_menu_btn_play_triggered(const QString& vm_id);
  void vbox_menu_btn_stop_triggered(const QString& vm_id);
  void vbox_menu_btn_add_triggered(const QString& vm_id);
  void vbox_menu_btn_rem_triggered(const QString& vm_id);
  void launch_Hub();
  void launch_ss_triggered();

  /*hub slots*/
  void environments_updated_sl(int rr);
  void balance_updated_sl();
  void got_ss_console_readiness_sl(bool is_ready, QString err);
  void hub_container_mi_triggered(const CEnvironment *env,
                               const CHubContainer *cont, void *action);
  void ssh_key_generate_triggered();
  void show_notifications_triggered();
  void ssh_to_container_finished(int result, void* additional_data);

  /*updater*/
  void update_available(QString file_id);
  void update_finished(QString file_id, bool success);
};
////////////////////////////////////////////////////////////////////////////
#endif // TRAYCONTROLWINDOW_H
