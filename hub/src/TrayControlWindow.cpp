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
#include "libssh2/LibsshErrors.h"
#include "HubController.h"
#include "RestWorker.h"
#include "DlgSettings.h"
#include "ApplicationLog.h"
#include "DlgAbout.h"
#include "DlgGenerateSshKey.h"
#include "HubComponentsUpdater.h"

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

  m_report_timer.setInterval(60*1000); //minute
  m_report_timer.start();

  connect(&m_refresh_timer, SIGNAL(timeout()),
          this, SLOT(refresh_timer_timeout()));

  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_add(const QString&)),
          this, SLOT(vm_added(const QString&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_remove(const QString&)),
          this, SLOT(vm_removed(const QString&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_state_changed(const QString&)),
          this, SLOT(vm_state_changed(const QString&)));

  connect(CNotifiactionObserver::Instance(), SIGNAL(notify(notification_level_t, const QString&)),
          this, SLOT(notification_received(notification_level_t, const QString&)));

  connect(&m_ss_updater_timer, SIGNAL(timeout()),
          this, SLOT(updater_timer_timeout()));

  connect(&CHubController::Instance(), SIGNAL(ssh_to_container_finished(int,void*)),
          this, SLOT(ssh_to_container_finished(int,void*)));

  connect(&m_report_timer, SIGNAL(timeout()), this, SLOT(report_timer_timeout()));
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
TrayControlWindow::add_vm_menu(const QString &vm_id) {
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
TrayControlWindow::remove_vm_menu(const QString &vm_id) {
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

void
TrayControlWindow::show_vbox() {
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);

  if (m_w_Player->vm_count > 0)
    m_vbox_menu->exec(curpos);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::create_tray_actions() {
  m_act_launch = new QAction(QIcon(":/hub/Launch-07.png") ,tr("Launch"), this);

  m_act_settings = new QAction(QIcon(":/hub/Settings-07.png"), tr("Settings"), this);
  connect(m_act_settings, SIGNAL(triggered()), this, SLOT(show_settings_dialog()));

  m_act_vbox = new QAction(QIcon(":/hub/VM-07.png"), tr("Virtual machines"), this);
  connect(m_act_vbox, SIGNAL(triggered()), this, SLOT(show_vbox()));

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

  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_settings);
  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_about);
  m_tray_menu->addAction(m_act_quit);
  //  m_tray_menu->addMenu(m_vbox_menu);

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
TrayControlWindow::vm_added(const QString &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(vboxAction);

  add_vm_menu(vm_id);
  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_removed(const QString &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(vboxAction);
  remove_vm_menu(vm_id);
  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_state_changed(const QString &vm_id) {
  auto ip = m_dct_player_menus.find(vm_id);
  if (ip == m_dct_player_menus.end()) return;
  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL) {
    //todo log
    return;
  }
  VM_State ns = vm->state();
  ip->second->set_buttons((ushort)ns);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_session_state_changed(const QString &vm_id) {
  UNUSED_ARG(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vmc_player_act_released(const QString &vm_id) { // remove
  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  bool on = (int)vm->state() == VMS_PoweredOff;

  if (on) {
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

void
TrayControlWindow::updater_timer_timeout() {
  m_ss_updater_timer.stop();  
  CHubComponentsUpdater::Instance()->subutai_rh_update();
  CHubComponentsUpdater::Instance()->tray_update();
  m_ss_updater_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::report_timer_timeout() {
  m_report_timer.stop();
  int http_code, err_code, network_err;
  CRestWorker::Instance()->send_health_request(http_code, err_code, network_err);
  m_report_timer.start();
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
TrayControlWindow::vbox_menu_btn_play_triggered(const QString& vm_id) {
  nsresult rc;
  const IVirtualMachine *vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  VM_State state = vm->state();
  if (state < 5) { //Powered off
    rc = CVBoxManagerSingleton::Instance()->launch_vm(vm_id);
    return;
  }

  if (state == VMS_Running) {
    rc = CVBoxManagerSingleton::Instance()->pause(vm_id);
    return;
  }

  if (state == VMS_Paused || state == VMS_Teleporting || state == VMS_LiveSnapshotting) {
    rc = CVBoxManagerSingleton::Instance()->resume(vm_id);
    return;
  }
  //todo use rc
  (void)rc;
  return;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_stop_triggered(const QString& vm_id) {
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
TrayControlWindow::vbox_menu_btn_add_triggered(const QString& vm_id) {
  //todo check result
  CVBoxManagerSingleton::Instance()->add(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_rem_triggered(const QString& vm_id) {
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
  DlgAbout dlg(this);
#ifdef RT_OS_LINUX
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250); //todo calculate it
  dlg.move(curpos.x(), 0);
#endif
  dlg.exec();
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
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  p_v_Layout->addLayout(p_h_HeaderLayout);
  this->setLayout(p_v_Layout);
}

///////////////////////////////////////////////////////////////////////////

CVBPlayerItem::CVBPlayerItem(const IVirtualMachine* vm, QWidget* parent) :
  m_vm_player_item_id(vm->id()) {

  UNUSED_ARG(parent);
  lbl_name = new QLabel(this);
  lbl_state = new QLabel(this);
  btn_play = new QPushButton("", this);
  btn_stop = new QPushButton("", this);

  p_h_Layout = new QHBoxLayout(NULL);  
  lbl_name->setMinimumWidth(180);
  lbl_state->setMinimumWidth(100);
  lbl_state->setMaximumWidth(100);

  ushort state = vm->state();

  lbl_name->setText(vm->name());
  lbl_state->setText(TrayControlWindow::GetStateName(state));

  btn_play->setIcon(QIcon(":/hub/Launch-07.png"));
  btn_stop->setIcon(QIcon(":/hub/Stop-07.png"));

  btn_play->setToolTip("Play/Pause/Resume");
  btn_stop->setToolTip("Power off");
  connect(btn_play, SIGNAL(released()),
          this, SLOT(vbox_menu_btn_play_released()), Qt::QueuedConnection);
  connect(btn_stop, SIGNAL(released()),
          this, SLOT(vbox_menu_btn_stop_released()), Qt::QueuedConnection);

  btn_remove = new QPushButton("", this);
  btn_remove->setIcon(QIcon(":/hub/Delete-07.png"));
  btn_remove->setToolTip("Attention! Removes VM. All files will be deleted");

  connect(btn_remove, SIGNAL(released()),
          this, SLOT(vbox_menu_btn_rem_released()), Qt::QueuedConnection);

  set_buttons(state);
  p_h_Layout->addWidget(lbl_name);
  p_h_Layout->addWidget(lbl_state);

  p_h_Layout->addWidget(btn_play);
  p_h_Layout->addWidget(btn_stop);
  p_h_Layout->addWidget(btn_remove);
  //p_h_Layout->addWidget(pAdd);

  p_h_Layout->setMargin(1);
  p_h_Layout->setSpacing(2);
  p_h_Layout->setObjectName(vm->name());
  this->setLayout(p_h_Layout);
}
////////////////////////////////////////////////////////////////////////////

CVBPlayerItem::~CVBPlayerItem(){
  p_h_Layout->removeWidget(lbl_name);
  p_h_Layout->removeWidget(lbl_state);

  disconnect(btn_play, SIGNAL(released()),
             this, SLOT(vbox_menu_btn_play_released()));
  disconnect(btn_stop, SIGNAL(released()),
             this, SLOT(vbox_menu_btn_stop_released()));

  p_h_Layout->removeWidget(btn_play);
  p_h_Layout->removeWidget(btn_stop);
}
////////////////////////////////////////////////////////////////////////////

void
CVBPlayerItem::set_buttons(ushort state) {
  struct layout_icons {
    QIcon play, stop, rem;
  };
  static layout_icons icon_set[] = {
    {QIcon(":/hub/Launch-07.png"), QIcon(":/hub/Stop_na-07.png"), QIcon(":/hub/Delete-07.png")},
    {QIcon(":/hub/Pause-07.png"), QIcon(":/hub/Stop-07.png"), QIcon(":/hub/Delete_na-07.png")},
    {QIcon(":/hub/Launch-07.png"), QIcon(":/hub/Stop-07.png"), QIcon(":/hub/Delete_na-07.png")},
    {QIcon(":/hub/Launch_na-07.png"), QIcon(":/hub/Stop-07.png"), QIcon(":/hub/Delete-07.png")},
    {QIcon(":/hub/Pause-07.png"), QIcon(":/hub/Stop_na-07.png"), QIcon(":/hub/Delete_na-07.png")},
    {QIcon(":/hub/Pause_na-07.png"), QIcon(":/hub/Stop_na-07.png"), QIcon(":/hub/Delete_na-07.png")}
  };

  lbl_state->setText(TrayControlWindow::GetStateName(state));
  int isi = 0;
  if (state < 5) isi = 0;
  else if (state == VMS_Running) isi = 1;
  else if (state == VMS_Paused) isi = 2;
  else if (state == VMS_Stuck) isi = 3;
  else if (state == VMS_Teleporting || state == VMS_LiveSnapshotting) isi = 4;
  else isi = 5; //state >= 10

  btn_play->setIcon(icon_set[isi].play);
  btn_stop->setIcon(icon_set[isi].stop);
  btn_remove->setIcon(icon_set[isi].rem);
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
  DlgGenerateSshKey dlg(this);
#ifdef RT_OS_LINUX
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  dlg.move(curpos.x(), 0);
#endif
  dlg.exec();
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
