#include <QMessageBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidgetAction>

#include "TrayControlWindow.h"
#include "ui_TrayControlWindow.h"
#include "DlgLogin.h"
#include "DlgSwarmJoin.h"
#include "IVBoxManager.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "libssh2/UpdateErrors.h"
#include "HubController.h"
#include "RestWorker.h"
#include "DlgSettings.h"
#include <QFileDialog>
#include <QDebug>

TrayControlWindow::TrayControlWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TrayControlWindow),
  m_hub_section(NULL),
  m_vbox_section(NULL),
  m_launch_section(NULL),
  m_quit_section(NULL),
  m_act_quit(NULL)
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
}

TrayControlWindow::~TrayControlWindow()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::fill_vm_menu(){
  if (CVBoxManagerSingleton::Instance()->init_machines() == 0) {
    for (auto i = CVBoxManagerSingleton::Instance()->dct_machines().begin();
         i != CVBoxManagerSingleton::Instance()->dct_machines().end(); ++i) {
#ifdef RT_OS_DARWIN
      add_vm_menu_simple(i->first);
#else
      add_vm_menu(i->first);
#endif
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::fill_launch_menu(){
  m_act_launch_SS = new QAction(QIcon(":/hub/SS-07.png"), tr("Launch SS console"), this);
  connect(m_act_launch_SS, SIGNAL(triggered()), this, SLOT(launch_SS()));

  m_act_launch_Hub = new QAction(QIcon(":/hub/Hub-07.png"), tr("Launch Hub website"), this);
  connect(m_act_launch_Hub, SIGNAL(triggered()), this, SLOT(launch_Hub()));

  m_launch_menu->addAction(m_act_launch_SS);
  m_launch_menu->addAction(m_act_launch_Hub);

}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::add_vm_menu(const com::Bstr &vm_id) {
  const IVirtualMachine* vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL) return;

  CVBPlayerItem *pl = new CVBPlayerItem(CVBoxManagerSingleton::Instance()->vm_by_id(vm_id), this);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_play_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_play_triggered, Qt::QueuedConnection);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_stop_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_stop_triggered, Qt::QueuedConnection);

  // Needed if we decide to have add button
  //  connect(pl, &CVBPlayerItem::vbox_menu_btn_add_released_signal,
  //          this, &TrayControlWindow::vbox_menu_btn_add_triggered);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_rem_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_rem_triggered, Qt::QueuedConnection);
  m_w_Player->add(pl);
  m_dct_player_menus[vm_id] = pl;
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::remove_vm_menu(const com::Bstr &vm_id) {
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

void TrayControlWindow::add_vm_menu_simple(const com::Bstr &vm_id) {
  const IVirtualMachine* vm = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id);
  if (vm == NULL) return;

  CVboxMenu* menu = new CVboxMenu(vm, this);

  m_vbox_menu->insertAction(m_vbox_section,
                            menu->action());

  connect(menu, &CVboxMenu::vbox_menu_act_triggered,
          this, &TrayControlWindow::vmc_act_released);


  VM_State state = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state();
  if ((int)state < 5){
    menu->set_machine_stopped(TRUE);
  } else {
    menu->set_machine_stopped(FALSE);
  }

  m_dct_vm_menus[vm_id] = menu;
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::remove_vm_menu_simple(const com::Bstr &vm_id) {
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
  QIcon icon;
  QSize size;
  size.setWidth(32);
  size.setHeight(32);

  icon.addFile(":/hub/Launch-07.png", size);

  //m_act_launch = new QAction(QIcon(":/hub/Launch-07.png"),tr("Launch"), this);
  m_act_launch = new QAction(icon,tr("Launch"), this);
  connect(m_act_launch, SIGNAL(triggered()), this, SLOT(show_launch()));

  m_act_settings = new QAction(QIcon(":/hub/Settings-07.png"), tr("Settings"), this);
  connect(m_act_settings, SIGNAL(triggered()), this, SLOT(show_settings_dialog()));

  m_act_vbox = new QAction(QIcon(":/hub/VM-07.png"), tr("Virtual machines"), this);
  connect(m_act_vbox, SIGNAL(triggered()), this, SLOT(show_vbox()));

  m_act_hub = new QAction(QIcon(":/hub/Environmetns-07.png"), tr("Environments"), this);
  connect(m_act_hub, SIGNAL(triggered()), this, SLOT(show_hub()));

  m_act_quit = new QAction(QIcon(":/hub/Exit-07"), tr("Quit"), this);
  connect(m_act_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

  m_act_info = new QAction(QIcon(":/hub/Balance-07.png"), CHubController::Instance().balance(), this);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::create_tray_icon()
{
  m_tray_menu = new QMenu(this);

  m_sys_tray_icon = new QSystemTrayIcon(this);
  m_sys_tray_icon->setContextMenu(m_tray_menu);
  m_sys_tray_icon->setIcon(QIcon(":/hub/Tray_icon_set-07.png"));

//////////// Do not forget to remove defs after fixing on linux!/////////////////
#ifdef RT_OS_LINUX
  m_info_menu = new QMenu(m_tray_menu);
  m_hub_menu = new QMenu(m_tray_menu);
  m_vbox_menu = new QMenu(m_tray_menu);
  m_launch_menu = new QMenu(m_tray_menu);
  m_vbox_menu->setIcon(QIcon(":/hub/VM-07.png"));
#endif

#ifndef RT_OS_LINUX
//  m_info_menu =  m_tray_menu->addMenu(QIcon(":/hub/Balance-07.png"), CHubController::Instance().balance());
  m_info_menu = new QMenu(m_tray_menu);
  m_tray_menu->addAction(m_act_info);
  m_tray_menu->addSeparator();
  m_launch_menu = m_tray_menu->addMenu(tr("Launch"));
  m_launch_menu->setIcon(QIcon(":/hub/Launch-07.png"));
  m_hub_menu = m_tray_menu->addMenu(tr("Environments"));
  m_hub_menu->setIcon(QIcon(":/hub/Environmetns-07.png"));
  m_vbox_menu = m_tray_menu->addMenu(tr("Virtual machines"));
  m_vbox_menu->setIcon(QIcon(":/hub/VM-07.png"));

  m_tray_menu->insertSeparator(m_act_settings);
#endif

  fill_vm_menu();
  fill_launch_menu();

#ifndef RT_OS_DARWIN
//  QWidgetAction *wAction = new QWidgetAction(m_vbox_menu);
//  wAction->setDefaultWidget(m_w_Player);
//  m_vbox_menu->addAction(wAction);

  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);

#endif

  m_info_section = m_info_menu->addSection("");
  m_launch_section = m_launch_menu->addSection("");
  m_hub_section  = m_hub_menu->addSection("");
  m_vbox_section = m_vbox_menu->addSection("");

//  m_tray_menu->insertAction(m_act_settings, m_act_info);
#ifdef RT_OS_LINUX
  m_tray_menu->insertAction(m_act_settings, m_act_launch);
  m_tray_menu->addAction(m_act_hub);
  m_tray_menu->addAction(m_act_hub);
  m_tray_menu->addAction(m_act_vbox);

  m_tray_menu->insertAction(m_act_launch, m_act_info);
  m_tray_menu->insertSeparator(m_act_launch);
#endif
//  //Will be changed when info menu action added - show transactions history
//  m_info_menu = new QMenu(m_tray_menu);
//  m_tray_menu->addAction(m_act_info);
//  m_tray_menu->addSeparator();

  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_settings);
  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_quit);
//  m_tray_menu->addMenu(m_vbox_menu);

//  m_sys_tray_icon = new QSystemTrayIcon(this);
//  m_sys_tray_icon->setContextMenu(m_tray_menu);
//  m_sys_tray_icon->setIcon(QIcon(":/hub/tray.png"));

}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::show_vbox() {
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  //m_vbox_menu->popup(curpos,m_act_hub);
  m_vbox_menu->exec(curpos);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::show_settings_dialog() {
  this->show();
  DlgSettings dlg(this);
#ifdef RT_OS_LINUX
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  dlg.move(curpos.x(), 0);
#endif
  dlg.exec();
  this->hide();
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::show_hub() {
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  //m_hub_menu->popup(curpos,m_act_hub);
  m_hub_menu->exec(curpos);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::show_launch() {
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
//m_launch_menu->popup(QCursor::pos(),m_act_launch);
   m_launch_menu->exec(curpos);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::notification_received(notification_level_t level,
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
void TrayControlWindow::vm_added(const com::Bstr &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(vboxAction);
  add_vm_menu(vm_id);
  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vm_removed(const com::Bstr &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(vboxAction);
  remove_vm_menu(vm_id);
  vboxAction = new QWidgetAction(m_vbox_menu);
  vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vm_state_changed(const com::Bstr &vm_id) {
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
  VM_State ns = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state();
  ip->second->set_buttons((ushort)ns);
#endif

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

void TrayControlWindow::vmc_player_act_released(const com::Bstr &vm_id) { // remove
  //  if (m_player_menus.find(vm_id) == m_player_menus.end())
  //    return;

  bool on = (int)CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state() == 5 ;//== VMS_PoweredOff;

  if (on) {
    int lr = CVBoxManagerSingleton::Instance()->pause(vm_id);
    qDebug() << "pausing result : " << lr << "\n";
    return;
  } //turn on
}
////////////////////////////////////////////////////////////////////////////

/*** Refresh ***/
void TrayControlWindow::refresh_timer_timeout() {
  /*balance*/
  refresh_balance();
  refresh_environments();
  CHubController::Instance().refresh_containers();
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::hub_container_mi_triggered(const CSSEnvironment *env,
                                                   const CHubContainer *cont) {

  CHubController::Instance().ssh_to_container(env, cont);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::updater_timer_timeout() {
  m_ss_updater_timer.stop();
  int exit_code = 0;

  CSystemCallWrapper::run_ss_updater(CSettingsManager::Instance().updater_host().toStdString().c_str(),
                                     CSettingsManager::Instance().updater_port().toStdString().c_str(),
                                     CSettingsManager::Instance().updater_user().toStdString().c_str(),
                                     CSettingsManager::Instance().updater_pass().toStdString().c_str(),
                                     "subutai-update",
                                     exit_code);

  if (exit_code == RUE_SUCCESS) {
    CNotifiactionObserver::NotifyAboutInfo("Update command succesfull finished");
  } else {
    CNotifiactionObserver::NotifyAboutError(QString("Update command failed with exit code : %1").arg(exit_code));
  }
  m_ss_updater_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::launch_Hub() {
  QString browser = "/etc/alternatives/x-www-browser"; //default browser
  QString folder;
  QString hub_url;
  hub_url = "https://hub.subut.ai";
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
    CNotifiactionObserver::NotifyAboutError(QString("Launch hub website failed. Error code : %1").arg((int)err));
    return;
  }
  qDebug() << browser << "yes " << (int)err << "\n";
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::launch_SS() {
  QString browser; // "/etc/alternatives/x-www-browser";
  QString folder;
  QString hub_url;
  hub_url = "https://localhost:9999";
  QStringList args;

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
system_call_wrapper_error_t err = CSystemCallWrapper::fork_process(browser,
                                                                     args,
                                                                     folder);

  //system_call_wrapper_error_t err = CSystemCallWrapper::open_url(hub_url); //for default browser
  if (err != SCWE_SUCCESS) {
    CNotifiactionObserver::NotifyAboutError(QString("Run SS console failed. Error code : %1").arg((int)err));
    return;
  }
  qDebug() << browser << "yes " << (int)err << "\n";
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vbox_menu_btn_play_triggered(const com::Bstr& vm_id){
  nsresult rc;
  ushort state = (int)CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state();
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

void TrayControlWindow::vbox_menu_btn_stop_triggered(const com::Bstr& vm_id){
  nsresult rc;
  ushort state = (int)CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state();
  if (state < 5) {
    return;
  }

  rc = CVBoxManagerSingleton::Instance()->turn_off(vm_id);
  //todo check rc
  (void)rc;
  return;
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vbox_menu_btn_add_triggered(const com::Bstr& vm_id){
  //todo check result
  CVBoxManagerSingleton::Instance()->add(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::vbox_menu_btn_rem_triggered(const com::Bstr& vm_id){
  //todo check result
  CVBoxManagerSingleton::Instance()->remove(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::refresh_balance() {
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
    disconnect(*i, SIGNAL(action_triggered(CSSEnvironment*, CHubContainer*)),
               this, SLOT(hub_container_mi_triggered(CSSEnvironment*,CHubContainer*)));
    delete *i;
  }
  m_lst_hub_menu_items.clear();

  for (auto env = CHubController::Instance().lst_environments().cbegin();
       env != CHubController::Instance().lst_environments().cend(); ++env) {

    QMenu* env_menu = m_hub_menu->addMenu(env->name());
    for (auto cont = env->containers().cbegin(); cont != env->containers().cend(); ++cont) {
      QAction* act = new QAction(cont->name(), this);
      CHubEnvironmentMenuItem* item = new CHubEnvironmentMenuItem(&(*env), &(*cont));
      connect(act, SIGNAL(triggered()), item, SLOT(internal_action_triggered()));
      connect(item, SIGNAL(action_triggered(const CSSEnvironment*, const CHubContainer*)),
              this, SLOT(hub_container_mi_triggered(const CSSEnvironment*, const CHubContainer*)));
      env_menu->addAction(act);
    }
  }
}
////////////////////////////////////////////////////////////////////////////

//static const char *GetStateName(ushort st)
const QString TrayControlWindow::GetStateName(ushort st)
{
  switch (st)
  {
    case 0:                return "<null>";
    case 1:            return "PoweredOff";
    case 2:                 return "Saved";
    case 3:            return "Teleported";
    case 4:               return "Aborted";
    case 5:               return "Running";
    case 6:                return "Paused";
    case 7:        return "GuruMeditation";
    case 8:           return "Teleporting";
    case 9:      return "LiveSnapshotting";
    case 10:             return "Starting";
    case 11:             return "Stopping";
    case 12:               return "Saving";
    case 13:            return "Restoring";
    case 14:  return "TeleportingPausedVM";
    case 15:        return "TeleportingIn";
    case 16: return "FaultTolerantSyncing";
    case 17: return "DeletingSnapshotOnline";
    case 18: return "DeletingSnapshotPaused";
    case 19:    return "OnlineSnapshotting";
    case 20:     return "RestoringSnapshot";
    case 21:      return "DeletingSnapshot";
    case 22:             return "SettingUp";
    case 23:          return "Snapshotting";
    default:              return "no idea";
  }
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

void CVboxMenu::set_machine_stopped(bool stopped) {
  QString str_icon = stopped ? ":/hub/Launch-07.png" : ":/hub/Stop-07.png";
  m_act->setIcon(QIcon(str_icon));
}
////////////////////////////////////////////////////////////////////////////

void CVboxMenu::act_triggered() {
  emit vbox_menu_act_triggered(m_id);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*hub menu*/
void CHubEnvironmentMenuItem::internal_action_triggered() {
  emit action_triggered(m_hub_environment, m_hub_container);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CVBPlayer::CVBPlayer(QWidget* parent) :
  m_vm_player_id() {
  UNUSED_ARG(parent);
  p_v_Layout = new QVBoxLayout(0);
  p_v_Layout->setSpacing(5);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  this->setLayout(p_v_Layout);
}

CVBPlayer::~CVBPlayer(){
}
////////////////////////////////////////////////////////////////////////////

void CVBPlayer::add(CVBPlayerItem* pItem){

  p_v_Layout->addWidget(pItem);
  int cnt = p_v_Layout->layout()->count();

  this->setMinimumHeight(35*(cnt+1));
  this->setMaximumHeight(35*(cnt+1));

  this->setLayout(p_v_Layout);
  this->setVisible(true);
}
////////////////////////////////////////////////////////////////////////////

void CVBPlayer::remove(CVBPlayerItem* pItem){

  p_v_Layout->removeWidget(pItem);
  int cnt = p_v_Layout->layout()->count();

  this->setMinimumHeight(35*(cnt+1));
  this->setMaximumHeight(35*(cnt+1));

  this->setLayout(p_v_Layout);
  this->setVisible(true);
}
///////////////////////////////////////////////////////////////////////////

CVBPlayerItem::CVBPlayerItem(const IVirtualMachine* vm, QWidget* parent) :
  m_vm_player_item_id(vm->id()) {

  UNUSED_ARG(parent);
  pLabelName = new QLabel(this);
  pLabelState = new QLabel(this); pPlay = new QPushButton("", this);
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

void CVBPlayerItem::set_buttons(ushort state){
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
void CVBPlayerItem::vbox_menu_btn_play_released() {
  emit(CVBPlayerItem::vbox_menu_btn_play_released_signal(m_vm_player_item_id));
}

void CVBPlayerItem::vbox_menu_btn_stop_released() {
  emit(CVBPlayerItem::vbox_menu_btn_stop_released_signal(m_vm_player_item_id));
}

void CVBPlayerItem::vbox_menu_btn_add_released() {
  emit(CVBPlayerItem::vbox_menu_btn_add_released_signal(m_vm_player_item_id));
}

void CVBPlayerItem::vbox_menu_btn_rem_released() {
  emit(CVBPlayerItem::vbox_menu_btn_rem_released_signal(m_vm_player_item_id));
}
///////////////////////////////////////////////////////////////////////////
