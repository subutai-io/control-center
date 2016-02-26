#include <QMessageBox>

#include "TrayControlWindow.h"
#include "ui_TrayControlWindow.h"
#include "DlgLogin.h"
#include "DlgSwarmJoin.h"
#include "IVBoxManager.h"
#include "IVBoxManager.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"

#include <QDebug>

TrayControlWindow::TrayControlWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TrayControlWindow),
  m_vbox_section(NULL),
  m_hub_section(NULL),
  m_quit_section(NULL),
  m_act_quit(NULL)
{
  ui->setupUi(this);
  create_tray_actions();
  create_tray_icon();
  m_sys_tray_icon->show();

  refresh_timer_timeout(); //update data on start. hack
  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec()*1000);
  m_refresh_timer.start();

  connect(&m_refresh_timer, SIGNAL(timeout()),
          this, SLOT(refresh_timer_timeout()));

  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_add(const com::Bstr&)),
          this, SLOT(vm_added(const com::Bstr&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_remove(const com::Bstr&)),
          this, SLOT(vm_removed(const com::Bstr&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_state_changed(const com::Bstr&)),
          this, SLOT(vm_state_changed(const com::Bstr&)));

  if (CVBoxManagerSingleton::Instance()->init_machines() == 0) {
    for (auto i = CVBoxManagerSingleton::Instance()->dct_machines().begin();
         i != CVBoxManagerSingleton::Instance()->dct_machines().end(); ++i) {
      add_vm_menu(i->first);
    }
  }  
}

TrayControlWindow::~TrayControlWindow()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::add_vm_menu(const com::Bstr &vm_id) {
  const IVirtualMachine* vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL) return;

  CVboxMenu* menu = new CVboxMenu(vm, this);
  m_vbox_menu->insertAction(m_vbox_section,
                            menu->action());
  connect(menu, SIGNAL(vbox_menu_act_triggered(const com::Bstr&)),
          this, SLOT(vmc_act_released(const com::Bstr&)));
  m_dct_vm_menus[vm_id] = menu;
}

void TrayControlWindow::remove_vm_menu(const com::Bstr &vm_id) {
  auto it = m_dct_vm_menus.find(vm_id);
  if (it == m_dct_vm_menus.end()) return;
  m_vbox_menu->removeAction(it->second->action());
  disconnect(it->second, SIGNAL(vbox_menu_act_triggered(const com::Bstr&)),
             this, SLOT(vmc_act_released(const com::Bstr&)));
  delete it->second;
  m_dct_vm_menus.erase(it);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::create_tray_actions()
{
  m_act_quit = new QAction(QIcon(":/hub/log_out"), tr("Quit"), this);
  connect(m_act_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::create_tray_icon()
{
  m_tray_menu = new QMenu(this);

  m_hub_menu = m_tray_menu->addMenu(tr("Environments"));
  m_vbox_menu = m_tray_menu->addMenu(tr("Virtual machines"));

  m_hub_section  = m_hub_menu->addSection("");
  m_vbox_section = m_vbox_menu->addSection("");

  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_quit);

  m_sys_tray_icon = new QSystemTrayIcon(this);
  m_sys_tray_icon->setContextMenu(m_tray_menu);
  m_sys_tray_icon->setIcon(QIcon(":/hub/tray.png"));
}
////////////////////////////////////////////////////////////////////////////

//void TrayControlWindow::show_hub() {
//  static bool logged_in = false;
//  if (!logged_in) {
//    logged_in = true;
//    m_hub_window.init_form();
//  }
//  m_hub_window.show();
//}
////////////////////////////////////////////////////////////////////////////

//void TrayControlWindow::join_to_swarm() {
//  DlgSwarmJoin dlg;
//  dlg.exec();
//}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/*** Vbox slots  ***/
void TrayControlWindow::vm_added(const com::Bstr &vm_id) {
  add_vm_menu(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vm_removed(const com::Bstr &vm_id) {
  remove_vm_menu(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vm_state_changed(const com::Bstr &vm_id) {
  auto it = m_dct_vm_menus.find(vm_id);
  if (it == m_dct_vm_menus.end()) return;
  VM_State ns = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state();
  it->second->set_machine_stopped(ns == VMS_Aborted || ns == VMS_PoweredOff);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vm_session_state_changed(const com::Bstr &vm_id) {
  UNUSED_ARG(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vmc_act_released(const com::Bstr &vm_id) {
  if (m_dct_vm_menus.find(vm_id) == m_dct_vm_menus.end())
    return;

  bool on = (int)CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state() < 5 ;//== VMS_PoweredOff;

  if (on) {
    int lr = CVBoxManagerSingleton::Instance()->launch_vm(vm_id);
    qDebug() << "launch result : " << lr;
    return;
  } //turn on

  //turn off
  int tor = CVBoxManagerSingleton::Instance()->turn_off(vm_id, false);
  if (!tor) return;
  //show_err(tor);
}
////////////////////////////////////////////////////////////////////////////

/*** Refresh ***/
void TrayControlWindow::refresh_timer_timeout() {
  m_refresh_timer.stop();
  int http_code, err_code;
  std::vector<CSSEnvironment> res = CRestWorker::get_environments(http_code, err_code);
  if (res == m_lst_environments) return;

  m_hub_menu->clear();
  for (auto i = m_lst_hub_menu_items.begin(); i != m_lst_hub_menu_items.end(); ++i) {
    disconnect(*i, SIGNAL(action_triggered(CSSEnvironment*, CHubContainer*)),
               this, SLOT(hub_menu_item_triggered(CSSEnvironment*,CHubContainer*)));
    delete *i;
  }
  m_lst_hub_menu_items.clear();
  m_lst_environments = res;

  for (auto env = m_lst_environments.begin(); env != m_lst_environments.end(); ++env) {
    QMenu* env_menu = m_hub_menu->addMenu(env->name());
    for (auto cont = env->containers().begin(); cont != env->containers().end(); ++cont) {
      QAction* act = new QAction(cont->name(), this);
      CHubEnvironmentMenuItem* item = new CHubEnvironmentMenuItem(&(*env), &(*cont));
      connect(act, SIGNAL(triggered()), item, SLOT(internal_action_triggered()));
      connect(item, SIGNAL(action_triggered(const CSSEnvironment*, const CHubContainer*)),
                 this, SLOT(hub_menu_item_triggered(const CSSEnvironment*, const CHubContainer*)));
      env_menu->addAction(act);
    }
  }

  m_refresh_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::hub_menu_item_triggered(const CSSEnvironment *env,
                                                const CHubContainer *cont) {
  bool connected_to_p2p = CSystemCallWrapper::join_to_p2p_swarm(env->hash().toStdString().c_str(),
                                                                env->key().toStdString().c_str(),
                                                                "10.10.10.15");
  if (!connected_to_p2p) {
    qDebug() << "not connected to p2p";
    return;
  }

  CSystemCallWrapper::run_ssh_in_terminal("ubuntu", cont->ip().toStdString().c_str());
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CVboxMenu::CVboxMenu(const IVirtualMachine *vm, QWidget* parent) :
  m_id(vm->id()) {
  QString name = QString::fromUtf16((ushort*)vm->name().raw());
  m_act = new QAction(QIcon(":/hub/play.png"), name, parent);
  connect(m_act, SIGNAL(triggered()), this, SLOT(act_triggered()));
}

CVboxMenu::~CVboxMenu() {

}

void CVboxMenu::set_machine_stopped(bool stopped) {
  QString str_icon = stopped ? ":/hub/play.png" : ":/hub/stop.png";
  m_act->setIcon(QIcon(str_icon));
}

void CVboxMenu::act_triggered() {
  emit vbox_menu_act_triggered(m_id);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*hub menu*/
void CHubEnvironmentMenuItem::internal_action_triggered() {
  emit action_triggered(m_hub_environment, m_hub_container);
}
