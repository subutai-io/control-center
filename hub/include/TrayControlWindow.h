#ifndef TRAYCONTROLWINDOW_H
#define TRAYCONTROLWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <VBox/com/string.h>
#include <QTimer>

#include "IVirtualMachine.h"
#include "HubStatisticWindow.h"
#include "RestWorker.h"


namespace Ui {
  class TrayControlWindow;
}

class CVboxMenu : public QObject {
  Q_OBJECT
private:
  com::Bstr m_id;
  QAction* m_act;

public:
  CVboxMenu(const IVirtualMachine *vm, QWidget *parent);
  virtual ~CVboxMenu();
  void set_machine_stopped(bool stopped);
  QAction* action() {return m_act;}

signals:
  void vbox_menu_act_triggered(const com::Bstr& vm_id);

private slots:
  void act_triggered();
};
////////////////////////////////////////////////////////////////////////////

class CHubEnvironmentMenuItem : public QObject {
  Q_OBJECT
private:
  const CSSEnvironment* m_hub_environment;
  const CHubContainer* m_hub_container;

public:
  explicit CHubEnvironmentMenuItem(const CSSEnvironment* env,
                                   const CHubContainer* cont) :
    m_hub_environment(env), m_hub_container(cont){}

  ~CHubEnvironmentMenuItem(){}

signals:
  void action_triggered(const CSSEnvironment*, const CHubContainer*);

public slots:
  void internal_action_triggered();
};
////////////////////////////////////////////////////////////////////////////

class TrayControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TrayControlWindow(QWidget *parent = 0);
  ~TrayControlWindow();

private:
  Ui::TrayControlWindow *ui;
  /*hub*/
//  HubStatisticWindow m_hub_window;
  QTimer m_refresh_timer;
  std::vector<CSSEnvironment> m_lst_environments;
  std::vector<CHubEnvironmentMenuItem*> m_lst_hub_menu_items;
  /*hub end*/

  /*vbox*/
  std::map<com::Bstr, CVboxMenu*> m_dct_vm_menus;
  void add_vm_menu(const com::Bstr &vm_id);
  void remove_vm_menu(const com::Bstr &vm_id);
  /*vbox end*/

  /*tray icon*/
  QMenu *m_hub_menu;
  QMenu *m_vbox_menu;
  QAction *m_hub_section;
  QAction *m_vbox_section;
  QAction *m_quit_section;

  QAction *m_act_quit;
  QSystemTrayIcon* m_sys_tray_icon;
  QMenu* m_tray_menu;  

  void create_tray_actions();
  void create_tray_icon();
  /*tray icon end*/

private slots:
  /*tray slots*/
//  void show_hub();
//  void join_to_swarm();

  /*virtualbox slots*/
  void vm_added(const com::Bstr& vm_id);
  void vm_removed(const com::Bstr& vm_id);
  void vm_state_changed(const com::Bstr& vm_id);
  void vm_session_state_changed(const com::Bstr& vm_id);
  void vmc_act_released(const com::Bstr& vm_id);

  /*hub slots*/
  void refresh_timer_timeout();
  void hub_menu_item_triggered(const CSSEnvironment *env,
                               const CHubContainer *cont);
};

#endif // TRAYCONTROLWINDOW_H
