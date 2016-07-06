#include <atomic>
#include <QMessageBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidgetAction>
#include <QFileDialog>
#include <QDir>
#include <QtConcurrent/QtConcurrent>

#include "TrayControlWindow.h"
#include "ui_TrayControlWindow.h"
#include "DlgLogin.h"
#include "IVBoxManager.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "libssh2/UpdateErrors.h"
#include "HubController.h"
#include "RestWorker.h"
#include "DlgSettings.h"
#include "ApplicationLog.h"
#include "DlgAbout.h"
#include "DlgGenerateSshKey.h"
#include "DownloadFileManager.h"
#include "ExecutableUpdater.h"

TrayControlWindow::TrayControlWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TrayControlWindow),
  m_hub_section(NULL),
  m_vbox_section(NULL),
  m_launch_section(NULL),
  m_info_section(NULL),
  m_quit_section(NULL),

  m_act_generate_ssh(NULL),
  m_act_quit(NULL),
  m_act_settings(NULL),
  m_act_info(NULL),
  m_act_vbox(NULL),
  m_act_hub(NULL),
  m_act_launch(NULL),
  m_act_launch_SS(NULL),
  m_act_launch_Hub(NULL),
  m_act_about(NULL),
  m_sys_tray_icon(NULL),
  m_tray_menu(NULL)
{
  ui->setupUi(this);
  m_w_Player = new CVBPlayer(this);

  create_tray_actions();
  create_tray_icon();
  m_sys_tray_icon->show();

  refresh_timer_timeout(); //update data on start. hack
  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec()*1000);
  m_refresh_timer.start();

  m_ss_updater_timer.setInterval(3*60*60*1000); //3 hours
  m_ss_updater_timer.start();

  connect(&m_refresh_timer, SIGNAL(timeout()),
          this, SLOT(refresh_timer_timeout()));

  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_add(const com::Bstr&)),
          this, SLOT(vm_added(const com::Bstr&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_remove(const com::Bstr&)),
          this, SLOT(vm_removed(const com::Bstr&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_state_changed(const com::Bstr&)),
          this, SLOT(vm_state_changed(const com::Bstr&)));

  connect(CNotifiactionObserver::Instance(), SIGNAL(notify(notification_level_t, const QString&)),
          this, SLOT(notification_received(notification_level_t, const QString&)));

  connect(&m_ss_updater_timer, SIGNAL(timeout()),
          this, SLOT(updater_timer_timeout()));

  connect(&CHubController::Instance(), SIGNAL(ssh_to_container_finished(int,void*)),
          this, SLOT(ssh_to_container_finished(int,void*)));
}

TrayControlWindow::~TrayControlWindow() {
  if (m_tray_menu) delete m_tray_menu;
  if (m_sys_tray_icon) delete m_sys_tray_icon;
  for (auto i = m_lst_hub_menu_items.begin(); i != m_lst_hub_menu_items.end(); ++i) {
    delete *i;
  }
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

int
TrayControlWindow::fill_vm_menu(){
  if (CVBoxManagerSingleton::Instance()->init_machines() != 0)
    return 0;
  ushort rh_count = 0;
  for (auto i = CVBoxManagerSingleton::Instance()->dct_machines().begin();
       i != CVBoxManagerSingleton::Instance()->dct_machines().end(); ++i) {
    add_vm_menu(i->first);
    rh_count++;
  }
  return rh_count;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::fill_launch_menu() {
  m_act_launch_SS = new QAction(QIcon(":/hub/SS-07.png"), tr("Launch SS console"), this);
  connect(m_act_launch_SS, SIGNAL(triggered()), this, SLOT(launch_ss_triggered()));

  m_act_launch_Hub = new QAction(QIcon(":/hub/Hub-07.png"), tr("Launch Hub website"), this);
  connect(m_act_launch_Hub, SIGNAL(triggered()), this, SLOT(launch_Hub()));

  m_launch_menu->addAction(m_act_launch_SS);
  m_launch_menu->addAction(m_act_launch_Hub);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::application_quit() {
  CCommons::QuitAppFlag = true;
  QApplication::quit();
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::add_vm_menu(const com::Bstr &vm_id) {
  const IVirtualMachine* vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL) return;

  CVBPlayerItem *pl = new CVBPlayerItem(CVBoxManagerSingleton::Instance()->vm_by_id(vm_id), this);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_play_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_play_triggered, Qt::QueuedConnection);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_stop_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_stop_triggered, Qt::QueuedConnection);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_rem_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_rem_triggered, Qt::QueuedConnection);
  m_w_Player->add(pl);
  m_dct_player_menus[vm_id] = pl;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::remove_vm_menu(const com::Bstr &vm_id) {
  auto it = m_dct_player_menus.find(vm_id);
  if (it == m_dct_player_menus.end()) return;  

  m_w_Player->remove(it->second);

  disconnect(it->second, &CVBPlayerItem::vbox_menu_btn_play_released_signal,
             this, &TrayControlWindow::vbox_menu_btn_play_triggered);

  connect(it->second, &CVBPlayerItem::vbox_menu_btn_stop_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_stop_triggered);

  delete it->second;
  m_dct_player_menus.erase(it);
}

////////////////////////////////////////////////////////////////////////////
////////// Delete it after approval! ///////////////////////////////////////
void
TrayControlWindow::add_vm_menu_simple(const com::Bstr &vm_id) {
  const IVirtualMachine* vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL) return;

  CVboxMenu* menu = new CVboxMenu(vm, this);

  m_vbox_menu->insertAction(m_vbox_section,
                            menu->action());

  connect(menu, &CVboxMenu::vbox_menu_act_triggered,
          this, &TrayControlWindow::vmc_act_released);

  VM_State state = vm->state();
  if ((int)state < 5){
    menu->set_machine_stopped(TRUE);
  } else {
    menu->set_machine_stopped(FALSE);
  }

  m_dct_vm_menus[vm_id] = menu;
}

////////////////////////////////////////////////////////////////////////////
////////// Delete it after approval! ///////////////////////////////////////
void
TrayControlWindow::remove_vm_menu_simple(const com::Bstr &vm_id) {
  auto it = m_dct_vm_menus.find(vm_id);
  if (it == m_dct_vm_menus.end()) return;
  m_vbox_menu->removeAction(it->second->action());
  disconnect(it->second, SIGNAL(vbox_menu_act_triggered(const com::Bstr&)),
             this, SLOT(vmc_act_released(const com::Bstr&)));
  delete it->second;
  m_dct_vm_menus.erase(it);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::create_tray_actions() {
  m_act_launch = new QAction(QIcon(":/hub/Launch-07.png") ,tr("Launch"), this);

  m_act_settings = new QAction(QIcon(":/hub/Settings-07.png"), tr("Settings"), this);
  connect(m_act_settings, SIGNAL(triggered()), this, SLOT(show_settings_dialog()));

  m_act_vbox = new QAction(QIcon(":/hub/VM-07.png"), tr("Virtual machines"), this);
  m_act_hub = new QAction(QIcon(":/hub/Environmetns-07.png"), tr("Environments"), this);

  m_act_quit = new QAction(QIcon(":/hub/Exit-07"), tr("Quit"), this);
  connect(m_act_quit, SIGNAL(triggered()), this, SLOT(application_quit()));

  m_act_info = new QAction(QIcon(":/hub/Balance-07.png"), CHubController::Instance().balance(), this);

  m_act_about = new QAction(QIcon(":/hub/about.png"), tr("About"), this);
  connect(m_act_about, SIGNAL(triggered()), this, SLOT(show_about()));

  m_act_generate_ssh = new QAction("Generate SSH key", this);
  connect(m_act_generate_ssh, SIGNAL(triggered()), this, SLOT(ssh_key_generate_triggered()));
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::create_tray_icon() {
  m_tray_menu = new QMenu(this);
  m_info_menu = new QMenu(m_tray_menu);
  m_tray_menu->addAction(m_act_info);
  m_tray_menu->addAction(m_act_generate_ssh);
  m_tray_menu->addSeparator();


  m_launch_menu = m_tray_menu->addMenu(tr("Launch"));
  m_launch_menu->setIcon(QIcon(":/hub/Launch-07.png"));
  m_hub_menu = m_tray_menu->addMenu(tr("Environments"));
  m_hub_menu->setIcon(QIcon(":/hub/Environmetns-07.png"));
#ifdef RT_OS_WINDOWS
  m_vbox_menu = m_tray_menu->addMenu(tr("Virtual machines"));
#else
  m_vbox_menu = new QMenu(m_tray_menu);
#endif

  m_vbox_menu->setIcon(QIcon(":/hub/VM-07.png"));

  fill_vm_menu();
  fill_launch_menu();

  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);

  m_info_section = m_info_menu->addSection("");
  m_launch_section = m_launch_menu->addSection("");
  m_hub_section  = m_hub_menu->addSection("");
  m_vbox_section = m_vbox_menu->addSection("");

  //  m_tray_menu->insertAction(m_act_settings, m_act_info);
#ifndef RT_OS_WINDOWS
  m_tray_menu->addAction(m_act_vbox);
#endif

  ////Will be changed when info menu action added - show transactions history
  //  m_info_menu = new QMenu(m_tray_menu);
  //  m_tray_menu->addAction(m_act_info);
  //  m_tray_menu->addSeparator();

  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_settings);
  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_about);
  m_tray_menu->addAction(m_act_quit);
  //  m_tray_menu->addMenu(m_vbox_menu);

  /////  Testing submenus, temporary, will be DELETED
  //  QMenu* temp_menu =  new QMenu("temorary test", m_tray_menu);
  //  temp_menu->addAction(m_act_quit);
  //  m_tray_menu->addMenu(temp_menu);

  m_sys_tray_icon = new QSystemTrayIcon(this);
  m_sys_tray_icon->setContextMenu(m_tray_menu);
  m_sys_tray_icon->setIcon(QIcon(":/hub/Tray_icon_set-07.png"));
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::show_settings_dialog() {
  DlgSettings dlg(this);
#ifdef RT_OS_LINUX
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  dlg.move(curpos.x(), 0);
#endif
  dlg.exec();
}
////////////////////////////////////////////////////////////////////////////


void
TrayControlWindow::notification_received(notification_level_t level,
                                         const QString &msg) {
  static const QString titles[] = {"Info", "Warning", "Error", "Critical"};

  //3rd element is warning, because critical shows messagebox when we need only notification.
  //we will show message box when critical errors appears
  static const QSystemTrayIcon::MessageIcon icons[] = {
    QSystemTrayIcon::Information, QSystemTrayIcon::Warning,
    QSystemTrayIcon::Warning, QSystemTrayIcon::Critical
  };
  m_sys_tray_icon->showMessage(titles[level],
                               msg,
                               icons[level],
                               CSettingsManager::Instance().notification_delay_sec() * 1000); //todo add delay to settings
}
////////////////////////////////////////////////////////////////////////////

/*** Vbox slots  ***/
void
TrayControlWindow::vm_added(const com::Bstr &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(vboxAction);

  add_vm_menu(vm_id);
  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_removed(const com::Bstr &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(vboxAction);
  remove_vm_menu(vm_id);
  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_state_changed(const com::Bstr &vm_id) {
#ifdef  RT_OS_DARWIN
  auto ip =  m_dct_vm_menus.find(vm_id);
  if (ip == m_dct_vm_menus.end()) return;
  VM_State ns = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state();
  if (ns < 5) {
    ip->second->set_machine_stopped(TRUE);
  } else {
    ip->second->set_machine_stopped(FALSE);
  }
#else
  auto ip = m_dct_player_menus.find(vm_id);
  if (ip == m_dct_player_menus.end()) return;
  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL) {
    //todo log
    return;
  }
  VM_State ns = vm->state();
  ip->second->set_buttons((ushort)ns);
#endif

}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_session_state_changed(const com::Bstr &vm_id) {
  UNUSED_ARG(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vmc_act_released(const com::Bstr &vm_id) {
  if (m_dct_vm_menus.find(vm_id) == m_dct_vm_menus.end())
    return;

  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;

  bool on = (int)vm->state() < 5 ;//== VMS_PoweredOff;

  if (on) {
    /*todo chack result. int lr = */
    CVBoxManagerSingleton::Instance()->launch_vm(vm_id);
    return;
  }

  //turn off
  int tor = CVBoxManagerSingleton::Instance()->turn_off(vm_id, false);
  if (!tor) return;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vmc_player_act_released(const com::Bstr &vm_id) { // remove
  //  if (m_player_menus.find(vm_id) == m_player_menus.end())
  //    return;

  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  bool on = (int)vm->state() == 5 ;//== VMS_PoweredOff;

  if (on) {
    /*int lr =*/
    CVBoxManagerSingleton::Instance()->pause(vm_id);
    return;
  } //turn on
}
////////////////////////////////////////////////////////////////////////////

/*** Refresh ***/
void
TrayControlWindow::refresh_timer_timeout() {  
  m_refresh_timer.stop();
  refresh_balance();
  refresh_environments();
  CHubController::Instance().refresh_containers();
  m_refresh_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::hub_container_mi_triggered(const CSSEnvironment *env,
                                              const CHubContainer *cont,
                                              void* action) {
  QAction* act = static_cast<QAction*>(action);
  if (act != NULL) {
    act->setEnabled(false);
    CHubController::Instance().ssh_to_container(env, cont, action);
  }
}
////////////////////////////////////////////////////////////////////////////

#if defined(RT_OS_LINUX)
#define UPDATE_FILE_TO_REPLACE "tray_9683ecfe-1034-11e6-b626-f816544befe7"
#elif defined(RT_OS_DARWIN)
#define UPDATE_FILE_TO_REPLACE "tray_9683ecfe-1034-11e6-b626-f816544befe7_mac"
#elif defined(RT_OS_WINDOWS)
#define UPDATE_FILE_TO_REPLACE "tray_9683ecfe-1034-11e6-b626-f816544befe7.exe"
#else
#error "UPDATE_FILE_TO_REPLACE macros undefined"
#endif

void
TrayControlWindow::updater_timer_timeout() {
  m_ss_updater_timer.stop();
  int exit_code = 0;

  /*subutai update*/
  CSystemCallWrapper::run_ss_updater(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_port().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_user().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                                     exit_code);

  if (exit_code == RLE_SUCCESS) {
    CNotifiactionObserver::NotifyAboutInfo("Update command succesfull finished");
    CApplicationLog::Instance()->LogInfo("Update command succesfull finished");
  } else {
    QString err_msg = QString("Update command failed with exit code : %1").arg(exit_code);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
  }

  /*tray update*/
  auto fi = CRestWorker::Instance()->get_gorjun_file_info(UPDATE_FILE_TO_REPLACE);
  if (!fi.empty()) {
    auto item = fi.begin();
    QString new_file_path = QApplication::applicationDirPath() +
                            QDir::separator() +
                            QString(UPDATE_FILE_TO_REPLACE);

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        new_file_path,
                                                        item->size());
    CExecutableUpdater *eu = new CExecutableUpdater(new_file_path,
                                                    QApplication::applicationFilePath());

    dm->start_download();
    connect(dm, SIGNAL(finished()), eu, SLOT(replace_executables()));
    connect(eu, SIGNAL(finished()), dm, SLOT(deleteLater()));
    connect(eu, SIGNAL(finished()), eu, SLOT(deleteLater()));
  }

  m_ss_updater_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::launch_Hub() {
  QString browser = "/etc/alternatives/x-www-browser"; //default browser
  QString folder;
  QString hub_url = "https://hub.subut.ai";
  QStringList args;

#if defined(RT_OS_LINUX)
  browser = "/usr/bin/google-chrome-stable";//need to be checked may be we can use default browser here
  args << "--new-window";
#elif defined(RT_OS_DARWIN)
  browser = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"; //need to be checked if need \ for spaces
#elif defined(RT_OS_WINDOWS)
  browser = "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
  folder = "C:\\Program Files (x86)\\Google\\Chrome\\Application";
  args << "--new-window";
#endif
  args << hub_url;
  system_call_wrapper_error_t err = CSystemCallWrapper::fork_process(
                                      browser,
                                      args,
                                      folder);

  //system_call_wrapper_error_t err = CSystemCallWrapper::open_url(hub_url);
  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Launch hub website failed. Error code : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    return;
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::launch_ss_triggered() {
  QAction* act = qobject_cast<QAction*>(sender());
  act->setEnabled(false);
  QtConcurrent::run(this, &TrayControlWindow::launch_ss, act);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_play_triggered(const com::Bstr& vm_id) {
  nsresult rc;
  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  ushort state = (int)vm->state();
  if (state < 5) { //Powered off
    rc = CVBoxManagerSingleton::Instance()->launch_vm(vm_id);
    return;
  }

  if (state == 5) { //Running
    rc = CVBoxManagerSingleton::Instance()->pause(vm_id);
    return;
  }

  if (state == 6 || state == 8 || state == 9) { //Paused
    rc = CVBoxManagerSingleton::Instance()->resume(vm_id);
    return;
  }
  //todo use rc
  (void)rc;
  return;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_stop_triggered(const com::Bstr& vm_id) {
  nsresult rc;
  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  ushort state = (int)vm->state();
  if (state < 5) {
    return;
  }

  rc = CVBoxManagerSingleton::Instance()->turn_off(vm_id);
  //todo check rc
  (void)rc;
  return;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_add_triggered(const com::Bstr& vm_id) {
  //todo check result
  CVBoxManagerSingleton::Instance()->add(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_rem_triggered(const com::Bstr& vm_id) {
  //todo check result
  CVBoxManagerSingleton::Instance()->remove(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::refresh_balance() {
  if (CHubController::Instance().refresh_balance()) return;
  m_act_info->setText(CHubController::Instance().balance());
  m_info_menu->setTitle(CHubController::Instance().balance());
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::refresh_environments() {
  if (CHubController::Instance().refresh_environments())
    return;

  m_hub_menu->clear();
  for (auto i = m_lst_hub_menu_items.begin(); i != m_lst_hub_menu_items.end(); ++i) {
    delete *i;
  }
  m_lst_hub_menu_items.clear();

  for (auto env = CHubController::Instance().lst_environments().cbegin();
       env != CHubController::Instance().lst_environments().cend(); ++env) {

    QString env_name = env->name();
    env_name.replace("_", "__"); //megahack :) Don't know how to handle underscores.
    QMenu* env_menu = m_hub_menu->addMenu(env_name);
    for (auto cont = env->containers().cbegin(); cont != env->containers().cend(); ++cont) {
      QAction* act = new QAction(cont->name(), this);
      CHubEnvironmentMenuItem* item = new CHubEnvironmentMenuItem(&(*env), &(*cont), m_sys_tray_icon);
      connect(act, SIGNAL(triggered()), item, SLOT(internal_action_triggered()));
      connect(item, SIGNAL(action_triggered(const CSSEnvironment*, const CHubContainer*, void*)),
              this, SLOT(hub_container_mi_triggered(const CSSEnvironment*, const CHubContainer*, void*)));
      env_menu->addAction(act);
      m_lst_hub_menu_items.push_back(item);
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::launch_ss(QAction* act) {
  QString browser; // "/etc/alternatives/x-www-browser";
  QString folder;
  QString hub_url;
  QStringList args;
  std::string rh_ip;
  int ec = 0;

  hub_url = "https://localhost:9999";

  system_call_wrapper_error_t err =
      CSystemCallWrapper::get_rh_ip_via_libssh2(
        CSettingsManager::Instance().rh_host().toStdString().c_str(),
        CSettingsManager::Instance().rh_port().toStdString().c_str(),
        CSettingsManager::Instance().rh_user().toStdString().c_str(),
        CSettingsManager::Instance().rh_pass().toStdString().c_str(),
        ec,
        rh_ip);

  if (err == SCWE_SUCCESS && ec == 0) {
    hub_url = QString("https://%1:8443").arg(rh_ip.c_str());
  } else {
    CApplicationLog::Instance()->LogError("Can't get RH IP address. Err : %s, exit_code : %d",
                                          run_libssh2_error_to_str((run_libssh2_error_t)err), ec);
    CNotifiactionObserver::Instance()->NotifyAboutInfo(QString("Can't get RH IP address. Error : %1, Exit_Code : %2").
                                                        arg(run_libssh2_error_to_str((run_libssh2_error_t)err)).
                                                        arg(ec));
    act->setEnabled(true);
    return; //todo check
  }

  int http_code, network_err, err_code;
  http_code = CRestWorker::Instance()->is_ss_console_ready(
                QString("https://%1:8443/rest/v1/peer/ready").arg(rh_ip.c_str()), err_code, network_err);

  if (network_err != 0 || err_code != 0 || http_code != 200) {
    QString err_msg;
    if (network_err == 0 && err_code == 0) {
      switch (http_code) {
        case 500:
          err_msg = "Some modules failed (SS restart might be needed)";
          break;
        case 503:
          err_msg = "Not ready yet/ loading";
          break;
        case 404:
          err_msg = "Endpoint itself not loaded yet (edited)";
          break;
        default:
          err_msg = QString("Undefined error. Code : %1").arg(http_code);
      }
    } else {
      err_msg = QString("Can't get SS console's status. Err : %1").arg(CRestWorker::rest_err_to_str((rest_error_t)err_code));
    }
    CNotifiactionObserver::Instance()->NotifyAboutInfo(err_msg);
    act->setEnabled(true);
    return;
  }

#if defined(RT_OS_LINUX)
  browser = "/usr/bin/google-chrome-stable";//need to be checked may be we can use default browser here
  args << "--new-window";
#elif defined(RT_OS_DARWIN)
  browser = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"; //need to be checked
#elif defined(RT_OS_WINDOWS)
  browser = "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
  folder = "C:\\Program Files (x86)\\Google\\Chrome\\Application";
  args << "--new-window";
#endif

  args << hub_url;
  err = CSystemCallWrapper::fork_process(browser,
                                         args,
                                         folder);

  //system_call_wrapper_error_t err = CSystemCallWrapper::open_url(hub_url); //for default browser
  if (err != SCWE_SUCCESS) {
    QString err_msg = QString("Run SS console failed. Error code : %1").
                      arg(CSystemCallWrapper::scwe_error_to_str(err));
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    act->setEnabled(true);
    return;
  }
  act->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::show_about() {
  //  this->show();
  DlgAbout dlg(this);
#ifdef RT_OS_LINUX
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  dlg.move(curpos.x(), 0);
#endif
  dlg.exec();
  //  this->hide();
}
////////////////////////////////////////////////////////////////////////////

const QString
TrayControlWindow::GetStateName(ushort st) {
  //here was switch, but this should be better.
  static const QString state_strings[] = {
    "<null>", "PoweredOff", "Saved",
    "Teleported", "Aborted", "Running",
    "Paused", "GuruMeditation", "Teleporting",
    "LiveSnapshotting", "Starting", "Stopping",
    "Saving", "Restoring", "TeleportingPausedVM",
    "TeleportingIn", "FaultTolerantSyncing", "DeletingSnapshotOnline",
    "DeletingSnapshotPaused", "OnlineSnapshotting", "RestoringSnapshot",
    "DeletingSnapshot", "SettingUp", "Snapshotting",
    "no idea",
  };
  return st >= (sizeof(state_strings)/sizeof(QString)) ?
        "no idea" : state_strings[st];
}
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

CVboxMenu::CVboxMenu(const IVirtualMachine *vm, QWidget* parent) :
  m_id(vm->id()) {
  QString name = QString::fromUtf16((ushort*)vm->name().raw());
  ushort state = (ushort)vm->state();
  // CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state();

  QString s_state = TrayControlWindow::GetStateName(state);
  //    m_act = new QAction(QIcon(":/hub/play.png"), name + " " + s_state, parent);
  m_act = new QAction(QIcon(":/hub/Launch-07.png"), name, parent);
  connect(m_act, SIGNAL(triggered()), this, SLOT(act_triggered()));
}

CVboxMenu::~CVboxMenu() {
}
////////////////////////////////////////////////////////////////////////////

void
CVboxMenu::set_machine_stopped(bool stopped) {
  QString str_icon = stopped ? ":/hub/Launch-07.png" : ":/hub/Stop-07.png";
  m_act->setIcon(QIcon(str_icon));
}
////////////////////////////////////////////////////////////////////////////

void
CVboxMenu::act_triggered() {
  emit vbox_menu_act_triggered(m_id);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*hub menu*/
void
CHubEnvironmentMenuItem::internal_action_triggered() {
  QAction* act = static_cast<QAction*>(sender());
  emit action_triggered(m_hub_environment, m_hub_container, (void*)act);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CVBPlayer::CVBPlayer(QWidget* parent) :
  m_vm_player_id() {
  UNUSED_ARG(parent);
  vm_count = 0;
  //  empty();
  labelHeader = new QLabel(this);
  labelHeader->setText("No resource hosts registered");
  labelHeader->setMinimumWidth(180);
  labelHeader->setSizePolicy(QSizePolicy::Preferred,
                             QSizePolicy::Minimum);
  p_v_Layout = new QVBoxLayout(0);
  p_v_Layout->setSpacing(2);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  p_v_Layout->addWidget(labelHeader);
  this->setLayout(p_v_Layout);
}

CVBPlayer::~CVBPlayer(){
}
////////////////////////////////////////////////////////////////////////////

void
CVBPlayer::add(CVBPlayerItem* pItem) {

  if (vm_count == 0){
    labelHeader->setText("Resource hosts registered:");
    labelHeader->setVisible(false);
  }
  p_v_Layout->addWidget(pItem);
  int cnt = p_v_Layout->layout()->count();

  this->setMinimumHeight(30*(cnt+1));
  this->setMaximumHeight(30*(cnt+1));

  vm_count++;
  this->setLayout(p_v_Layout);
  this->setVisible(true);  
}
////////////////////////////////////////////////////////////////////////////

void
CVBPlayer::remove(CVBPlayerItem* pItem) {

  p_v_Layout->removeWidget(pItem);
  int cnt = p_v_Layout->layout()->count();

  this->setMinimumHeight(30*(cnt+1));
  this->setMaximumHeight(30*(cnt+1));

  vm_count--;

  if (vm_count == 0){
    labelHeader->setText("No resource hosts registered:");
    labelHeader->setVisible(true);
  }
  this->setLayout(p_v_Layout);
  this->setVisible(true);
}
/////////////////////////////////////////////////////////////////////////////

void
CVBPlayer::empty() {
  labelHeader = new QLabel(this);
  labelHeader->setText("No resource hosts registered");
  labelHeader->setMinimumWidth(180);
  p_h_HeaderLayout = new QHBoxLayout(this);
  p_h_HeaderLayout->addWidget(labelHeader);
  p_v_Layout = new QVBoxLayout(0);
  p_v_Layout->setSpacing(5);
  //  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  p_v_Layout->addLayout(p_h_HeaderLayout);
  this->setLayout(p_v_Layout);
}

///////////////////////////////////////////////////////////////////////////

CVBPlayerItem::CVBPlayerItem(const IVirtualMachine* vm, QWidget* parent) :
  m_vm_player_item_id(vm->id()) {

  UNUSED_ARG(parent);
  pLabelName = new QLabel(this);
  pLabelState = new QLabel(this);
  pPlay = new QPushButton("", this);
  pStop = new QPushButton("", this);

  p_h_Layout = new QHBoxLayout(NULL);
  QString name = QString::fromUtf16((ushort*)vm->name().raw());
  pLabelName->setMinimumWidth(180);
  pLabelState->setMinimumWidth(100);
  pLabelState->setMaximumWidth(100);

  ushort state = (ushort)vm->state();

  pLabelName->setText(name);
  pLabelState->setText(TrayControlWindow::GetStateName(state));

  pPlay->setIcon(QIcon(":/hub/Launch-07.png"));
  pStop->setIcon(QIcon(":/hub/Stop-07.png"));

  pPlay->setToolTip("Play/Pause/Resume");
  pStop->setToolTip("Power off");
  connect(pPlay, SIGNAL(released()),
          this, SLOT(vbox_menu_btn_play_released()), Qt::QueuedConnection);
  connect(pStop, SIGNAL(released()),
          this, SLOT(vbox_menu_btn_stop_released()), Qt::QueuedConnection);

  //  pAdd = new QPushButton("Add", this);
  //  pAdd->setIcon(QIcon(":/hub/play.png"));
  //  connect(pAdd, SIGNAL(released()),
  //          this, SLOT(vbox_menu_btn_add_released()));

  pRem = new QPushButton("", this);
  pRem->setIcon(QIcon(":/hub/Delete-07.png"));
  pRem->setToolTip("Attention! Removes VM. All files will be deleted");


  connect(pRem, SIGNAL(released()),
          this, SLOT(vbox_menu_btn_rem_released()), Qt::QueuedConnection);

  set_buttons(state);
  p_h_Layout->addWidget(pLabelName);
  p_h_Layout->addWidget(pLabelState);

  p_h_Layout->addWidget(pPlay);
  p_h_Layout->addWidget(pStop);
  p_h_Layout->addWidget(pRem);
  //p_h_Layout->addWidget(pAdd);

  p_h_Layout->setMargin(1);
  p_h_Layout->setSpacing(2);
  p_h_Layout->setObjectName(name);
  this->setLayout(p_h_Layout);
}
////////////////////////////////////////////////////////////////////////////

CVBPlayerItem::~CVBPlayerItem(){
  p_h_Layout->removeWidget(pLabelName);
  p_h_Layout->removeWidget(pLabelState);

  disconnect(pPlay, SIGNAL(released()),
             this, SLOT(vbox_menu_btn_play_released()));
  disconnect(pStop, SIGNAL(released()),
             this, SLOT(vbox_menu_btn_stop_released()));

  p_h_Layout->removeWidget(pPlay);
  p_h_Layout->removeWidget(pStop);
}
////////////////////////////////////////////////////////////////////////////

void
CVBPlayerItem::set_buttons(ushort state) {
  if (state < 5) { //turned off
    pLabelState->setText(TrayControlWindow::GetStateName(state));
    pPlay->setIcon(QIcon(":/hub/Launch-07.png"));
    pStop->setIcon(QIcon(":/hub/Stop_na-07.png"));
    pRem->setIcon(QIcon(":/hub/Delete-07.png"));
    return;
  }

  if (state == 5) {
    pLabelState->setText(TrayControlWindow::GetStateName(state));
    pPlay->setIcon(QIcon(":/hub/Pause-07.png"));
    pStop->setIcon(QIcon(":/hub/Stop-07.png"));
    pRem->setIcon(QIcon(":/hub/Delete_na-07.png"));
    return;
  }
  if (state == 6) {//Paused
    pLabelState->setText(TrayControlWindow::GetStateName(state));
    pPlay->setIcon(QIcon(":/hub/Launch-07.png"));
    pStop->setIcon(QIcon(":/hub/Stop-07.png"));
    pRem->setIcon(QIcon(":/hub/Delete_na-07.png"));
    return;
  }
  if (state == 7) {//Stuck, Gurumeditation, only power off
    pLabelState->setText(TrayControlWindow::GetStateName(state));
    pPlay->setIcon(QIcon(":/hub/Launch_na-07.png"));//Change to Play_Disabled
    pStop->setIcon(QIcon(":/hub/Stop-07.png"));
    pRem->setIcon(QIcon(":/hub/Delete-07.png"));
    return;
  }
  if (state == 8 || state == 9) {//Teleporting or LiveSnapshotting
    pLabelState->setText(TrayControlWindow::GetStateName(state));
    //str_icon = ":/hub/PauseHot.png";
    pPlay->setIcon(QIcon(":/hub/Pause-07.png"));
    pStop->setIcon(QIcon(":/hub/Stop_na-07.png"));
    pRem->setIcon(QIcon(":/hub/Delete_na-07.png"));
    return;
  }

  if (state >= 10 && state < 24) {//Teleporting or LiveSnapshotting
    //str_icon = ":/hub/PauseHot.png";
    pPlay->setIcon(QIcon(":/hub/Pause_na-07.png"));
    pStop->setIcon(QIcon(":/hub/Stop_na-07.png"));
    pRem->setIcon(QIcon(":/hub/Delete_na-07.png"));
    return;
  }
}

//Slots////////////////////////////////////////////////////////////////////
void
CVBPlayerItem::vbox_menu_btn_play_released() {
  emit(CVBPlayerItem::vbox_menu_btn_play_released_signal(m_vm_player_item_id));
}

void
CVBPlayerItem::vbox_menu_btn_stop_released() {
  emit(CVBPlayerItem::vbox_menu_btn_stop_released_signal(m_vm_player_item_id));
}

void
CVBPlayerItem::vbox_menu_btn_add_released() {
  emit(CVBPlayerItem::vbox_menu_btn_add_released_signal(m_vm_player_item_id));
}

void
CVBPlayerItem::vbox_menu_btn_rem_released() {
  emit(CVBPlayerItem::vbox_menu_btn_rem_released_signal(m_vm_player_item_id));
}
///////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::ssh_key_generate_triggered() {
  //  this->show();
  DlgGenerateSshKey dlg(this);
#ifdef RT_OS_LINUX
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  dlg.move(curpos.x(), 0);
#endif
  dlg.exec();
  //  this->hide();
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::ssh_to_container_finished(int result,
                                             void *additional_data) {
  if (result != SLE_SUCCESS) {
    CNotifiactionObserver::Instance()->NotifyAboutError(
          QString("Can't ssh to container. Err : %1").arg(CHubController::ssh_launch_err_to_str(result)));
  }
  QAction* act = static_cast<QAction*>(additional_data);
  if (act == NULL) return;
  act->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////
