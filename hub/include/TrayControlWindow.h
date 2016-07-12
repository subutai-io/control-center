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

#include "IVirtualMachine.h"
#include "RestWorker.h"
#include "NotifiactionObserver.h"

namespace Ui {
  class TrayControlWindow;
}

class CVBPlayerItem : public QWidget {
    Q_OBJECT
private:
    QString m_vm_player_item_id;
    QAction* m_player_item_act;
public:
    CVBPlayerItem(const IVirtualMachine* vm, QWidget* parent);
    virtual ~CVBPlayerItem();
    void set_buttons(ushort state);
    QAction* action() {return m_player_item_act;}
    QHBoxLayout *p_h_Layout;
    QLabel *pLabelName;
    QLabel *pLabelState;
    QPushButton *pPlay;
    QPushButton *pStop;
    QPushButton *pAdd;
    QPushButton *pRem;

signals:
    void vbox_menu_btn_play_released_signal(const QString& vm_id);
    void vbox_menu_btn_stop_released_signal(const QString& vm_id);
    void vbox_menu_btn_add_released_signal(const QString& vm_id);
    void vbox_menu_btn_rem_released_signal(const QString& vm_id);

public slots:
    void vbox_menu_btn_play_released();
    void vbox_menu_btn_stop_released();
    void vbox_menu_btn_add_released();
    void vbox_menu_btn_rem_released();
};
////////////////////////////////////////////////////////////////////////////

class CVBPlayer : public QWidget{
    Q_OBJECT
private:
    QString m_vm_player_id;
    QVBoxLayout *p_v_Layout;
    QAction* m_player_act;
    QLabel *labelHeader;
    QHBoxLayout *p_h_HeaderLayout;
    void empty();

public:
    CVBPlayer(QWidget *parent);
    virtual ~CVBPlayer();
    void add(CVBPlayerItem* pItem);
    void remove(CVBPlayerItem* pItem);
    int vm_count;
};
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class CHubEnvironmentMenuItem : public QObject {
 Q_OBJECT
private:
  const CSSEnvironment* m_hub_environment;
  QSystemTrayIcon* m_tray_icon;
  const CHubContainer* m_hub_container;

public:
  CHubEnvironmentMenuItem(const CSSEnvironment* env,
                          const CHubContainer* cont,
                          QSystemTrayIcon* tray_icon) :
    m_hub_environment(env), m_tray_icon(tray_icon), m_hub_container(cont) {}

  ~CHubEnvironmentMenuItem(){}

signals:
  void action_triggered(const CSSEnvironment*, const CHubContainer*, void* action);

public slots:
  void internal_action_triggered();

};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class TrayControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TrayControlWindow(QWidget *parent = 0);
  ~TrayControlWindow();
  static const QString GetStateName(ushort st);
  CVBPlayer *m_w_Player;

private:
  Ui::TrayControlWindow *ui;
  QVBoxLayout *m_w_Layout;
//  CVBPlayer *m_w_Player;
  /*hub*/
  QTimer m_refresh_timer;
  QTimer m_ss_updater_timer;
  QTimer m_report_timer;
  std::vector<CHubEnvironmentMenuItem*> m_lst_hub_menu_items;
  /*hub end*/

  /*vbox*/
  QWidgetAction *vboxAction;
  std::map<QString, CVBPlayerItem*>  m_dct_player_menus;
  void add_vm_menu(const QString &vm_id);
  void remove_vm_menu(const QString &vm_id);
  /*vbox end*/

  /*tray icon*/
  QMenu *m_hub_menu;
  QMenu *m_vbox_menu;
  QMenu *m_player_menu;
  QMenu *m_launch_menu;
  QMenu *m_info_menu;

  QAction *m_hub_section;
  QAction *m_vbox_section;
  QAction *m_launch_section;
  QAction *m_info_section;
  QAction *m_quit_section;

  QAction *m_act_generate_ssh;
  QAction *m_act_quit;
  QAction *m_act_settings;
  QAction *m_act_info;
  QAction *m_act_vbox;
  QAction *m_act_hub;
  QAction *m_act_launch;
  QAction *m_act_launch_SS;
  QAction *m_act_launch_Hub;

  QAction *m_act_about;

  QSystemTrayIcon* m_sys_tray_icon;
  QMenu* m_tray_menu;  

  void create_tray_actions();
  void create_tray_icon();
  int fill_vm_menu();
  void fill_launch_menu();  
  int IconPlace[4], TrayPlace[4], VboxPlace[4];
  /*tray icon end*/

  void refresh_balance();
  void refresh_environments();
  void launch_ss(QAction *act);
private slots:
  /*tray slots*/
  void show_about();
  void application_quit();
  void show_settings_dialog();  \
  void notification_received(notification_level_t level,
                             const QString& msg);

  /*virtualbox slots*/
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
  void refresh_timer_timeout();
  void hub_container_mi_triggered(const CSSEnvironment *env,
                               const CHubContainer *cont, void *action);
  void ssh_key_generate_triggered();
  void ssh_to_container_finished(int result, void* additional_data);

  /*updater*/
  void updater_timer_timeout();  
  void report_timer_timeout();
};
////////////////////////////////////////////////////////////////////////////

#endif // TRAYCONTROLWINDOW_H


