#include <QMessageBox>

#include "TrayControlWindow.h"
#include "ui_TrayControlWindow.h"
#include "DlgLogin.h"
#include "IVBoxManager.h"
#include "DlgSwarmJoin.h"

#include <QDebug>

TrayControlWindow::TrayControlWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TrayControlWindow),
  m_act_quit(NULL),
  m_act_hub(NULL),
  m_act_vbox(NULL),
  m_act_swarm_join(NULL)
{
  ui->setupUi(this);
  create_tray_actions();
  create_tray_icon();
  m_sys_tray_icon->show();
  show_hub();
}

TrayControlWindow::~TrayControlWindow()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::create_tray_actions()
{
  m_act_quit = new QAction(QIcon(":/hub/log_out"), tr("Quit"), this);
  connect(m_act_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

  m_act_hub = new QAction(QIcon(":/hub/tray.png"), tr("Show Hub"), this);
  connect(m_act_hub, SIGNAL(triggered()), this, SLOT(show_hub()));

  m_act_vbox = new QAction(QIcon(":/hub/virtual.png"), tr("Show Virtual Machines"), this);
  connect(m_act_vbox, SIGNAL(triggered()), this, SLOT(show_vbox()));

  m_act_swarm_join = new QAction(QIcon(":/hub/join_swarm.png"), tr("Join to swarm"), this);
  connect(m_act_swarm_join, SIGNAL(triggered()), this, SLOT(join_to_swarm()));
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::create_tray_icon()
{
  m_tray_menu = new QMenu(this);
  m_tray_menu->addAction(m_act_hub);
  m_tray_menu->addAction(m_act_vbox);
  m_tray_menu->addAction(m_act_swarm_join);

  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_quit);

  m_sys_tray_icon = new QSystemTrayIcon(this);
  m_sys_tray_icon->setContextMenu(m_tray_menu);
  m_sys_tray_icon->setIcon(QIcon(":/hub/tray.png"));

}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::show_hub() {
  static bool logged_in = false;
  if (!logged_in) {
    logged_in = true;
    m_hub_window.init_form();
  }
  m_hub_window.show();
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::show_vbox() {
  static bool vbox_initialized = false;
  if (!vbox_initialized) {
    if (int im = CVBoxManagerSingleton::Instance()->init_machines()) {
      QMessageBox msg_box(QMessageBox::Critical, "Initialization failed!!!",
                          QString("Sorry, error occured. Error code : %1. ").
                          arg(CVBoxManagerSingleton::Instance()->last_vb_error()) + QString(CCommons::VM_error_to_str(im)),
                          QMessageBox::Ok);
      msg_box.exec();
      return;
    }
    vbox_initialized = true;
    m_vbox_window.init_machines();
  }
  m_vbox_window.show();
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::join_to_swarm() {
  DlgSwarmJoin dlg;
  dlg.exec();
}
////////////////////////////////////////////////////////////////////////////
