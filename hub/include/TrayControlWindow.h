#ifndef TRAYCONTROLWINDOW_H
#define TRAYCONTROLWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "HubStatisticWindow.h"
#include "VBoxManageWindow.h"

namespace Ui {
  class TrayControlWindow;
}

class TrayControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TrayControlWindow(QWidget *parent = 0);
  ~TrayControlWindow();

private:
  Ui::TrayControlWindow *ui;
  /*hub*/
  HubStatisticWindow m_hub_window;
  /*hub end*/

  /*vbox*/
  VBoxManageWindow m_vbox_window;
  /*vbox end*/

  /*tray icon*/
  QAction *m_act_quit;
  QAction *m_act_hub;
  QAction *m_act_vbox;
  QAction *m_act_swarm_join;

  QSystemTrayIcon* m_sys_tray_icon;
  QMenu* m_tray_menu;  

  void create_tray_actions();
  void create_tray_icon();
  /*tray icon end*/

private slots:
  void show_hub();
  void show_vbox();
  void join_to_swarm();
};

#endif // TRAYCONTROLWINDOW_H
