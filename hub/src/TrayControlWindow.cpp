#include <algorithm>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidgetAction>
#include <QtConcurrent/QtConcurrent>
#include <QtGui>

#include "TrayControlWindow.h"
#include "ui_TrayControlWindow.h"
#include "DlgLogin.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "libssh2/include/LibsshController.h"
#include "HubController.h"
#include "RestWorker.h"
#include "DlgSettings.h"
#include "ApplicationLog.h"
#include "DlgAbout.h"
#include "DlgGenerateSshKey.h"
#include "updater/HubComponentsUpdater.h"
#include "DlgNotifications.h"
#include "VBoxManager.h"
#include "OsBranchConsts.h"

using namespace update_system;

TrayControlWindow::TrayControlWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TrayControlWindow),
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
  m_act_logout(NULL),
  m_sys_tray_icon(NULL),
  m_tray_menu(NULL)
{
  ui->setupUi(this);
  m_w_Player = new CVBPlayer(this);

  create_tray_actions();
  create_tray_icon();
  m_sys_tray_icon->show();

  QThread* th = new QThread;

  connect(CVboxManager::Instance(), &CVboxManager::vm_add,
          this, &TrayControlWindow::vm_added);
  connect(CVboxManager::Instance(), &CVboxManager::vm_remove,
          this, &TrayControlWindow::vm_removed);
  connect(CVboxManager::Instance(), &CVboxManager::vm_state_changed,
          this, &TrayControlWindow::vm_state_changed);
  connect(CVboxManager::Instance(), &CVboxManager::initialized,
          this, &TrayControlWindow::fill_vm_menu);

  connect(th, &QThread::started, CVboxManager::Instance(), &CVboxManager::start_work);
  connect(CVboxManager::Instance(), &CVboxManager::finished, th, &QThread::quit);
  connect(th, &QThread::finished, th, &QThread::deleteLater);
  CVboxManager::Instance()->moveToThread(th);
  th->start();

  connect(CNotificationObserver::Instance(), &CNotificationObserver::notify,
          this, &TrayControlWindow::notification_received);

  connect(&CHubController::Instance(), &CHubController::ssh_to_container_finished,
          this, &TrayControlWindow::ssh_to_container_finished);
  connect(&CHubController::Instance(), &CHubController::balance_updated,
          this, &TrayControlWindow::balance_updated_sl);
  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &TrayControlWindow::environments_updated_sl);

  connect(CRestWorker::Instance(), &CRestWorker::on_got_ss_console_readiness,
          this, &TrayControlWindow::got_ss_console_readiness_sl);

  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::updating_finished,
          this, &TrayControlWindow::update_finished);
  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::update_available,
          this, &TrayControlWindow::update_available);

  CHubController::Instance().force_refresh();
  login_success();
}

TrayControlWindow::~TrayControlWindow() {
  for (auto i = m_lst_hub_menu_items.begin(); i != m_lst_hub_menu_items.end(); ++i) {
    delete *i;
  }

  QMenu *menus[] = {m_hub_menu, m_vbox_menu, m_launch_menu, m_tray_menu, nullptr};
  QAction *acts[] = { m_act_generate_ssh, m_act_quit,
                      m_act_settings, m_act_info, m_act_vbox,
                      m_act_hub, m_act_launch, m_act_launch_SS,
                      m_act_launch_Hub, m_act_about, m_act_logout,
                      m_act_notifications_history, nullptr};

  for (int i = 0; menus[i]; ++i) {
    try { delete menus[i]; }
    catch (...) {}
  }

  for (int i = 0; acts[i]; ++i) {
    try { delete acts[i]; }
    catch (...) {}
  }

  try { delete m_sys_tray_icon; }
  catch (...) {}
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::fill_vm_menu(){
  for (auto i = CVboxManager::Instance()->dct_machines().begin();
       i != CVboxManager::Instance()->dct_machines().end(); ++i) {
    add_vm_menu(i->first);
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::fill_launch_menu() {
  m_act_launch_SS = new QAction(QIcon(":/hub/SS-07.png"), tr("Launch SS console"), this);
  connect(m_act_launch_SS, &QAction::triggered, this, &TrayControlWindow::launch_ss_triggered);

  m_act_launch_Hub = new QAction(QIcon(":/hub/Hub-07.png"), tr("Launch Hub website"), this);
  connect(m_act_launch_Hub, &QAction::triggered, this, &TrayControlWindow::launch_Hub);

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
  const CVirtualMachine* vm = CVboxManager::Instance()->vm_by_id(vm_id);
  if (vm == NULL) return;

  CVBPlayerItem *pl = new CVBPlayerItem(vm, m_w_Player);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_play_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_play_triggered, Qt::QueuedConnection);

  connect(pl, &CVBPlayerItem::vbox_menu_btn_stop_released_signal,
          this, &TrayControlWindow::vbox_menu_btn_stop_triggered, Qt::QueuedConnection);

  m_w_Player->add(pl);
  m_dct_player_menus[vm_id] = pl;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::remove_vm_menu(const QString &vm_id) {
  auto it = m_dct_player_menus.find(vm_id);
  if (it == m_dct_player_menus.end()) return;
  m_w_Player->remove(it->second);
  delete it->second;
  m_dct_player_menus.erase(it);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::show_vbox() {
  QPoint curpos = QCursor::pos();
  curpos.setX(curpos.x() - 250);
  if (m_w_Player->vm_count() > 0)
    m_vbox_menu->exec(curpos);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::create_tray_actions() {
  m_act_launch = new QAction(QIcon(":/hub/Launch-07.png") ,tr("Launch"), this);

  m_act_settings = new QAction(QIcon(":/hub/Settings-07.png"), tr("Settings"), this);
  connect(m_act_settings, &QAction::triggered, this, &TrayControlWindow::show_settings_dialog);

  m_act_vbox = new QAction(QIcon(":/hub/VM-07.png"), tr("Virtual machines"), this);
  connect(m_act_vbox, &QAction::triggered, this, &TrayControlWindow::show_vbox);

  m_act_hub = new QAction(QIcon(":/hub/Environmetns-07.png"), tr("Environments"), this);

  m_act_quit = new QAction(QIcon(":/hub/Exit-07"), tr("Quit"), this);
  connect(m_act_quit, &QAction::triggered, this, &TrayControlWindow::application_quit);

  m_act_info = new QAction(QIcon(":/hub/Balance-07.png"), CHubController::Instance().balance(), this);

  m_act_about = new QAction(QIcon(":/hub/about.png"), tr("About"), this);
  connect(m_act_about, &QAction::triggered, this, &TrayControlWindow::show_about);

  m_act_generate_ssh = new QAction(tr("Generate SSH key"), this);
  connect(m_act_generate_ssh, &QAction::triggered, this, &TrayControlWindow::ssh_key_generate_triggered);

  m_act_logout = new QAction(QIcon(":/hub/logout.png"), tr("Logout"), this);
  connect(m_act_logout, &QAction::triggered, this, &TrayControlWindow::logout);

  m_act_notifications_history = new QAction(QIcon(":hub/notifications_history.png"), "Notifications history", this);
  connect(m_act_notifications_history, &QAction::triggered,
          this, &TrayControlWindow::show_notifications_triggered);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::create_tray_icon() {

  m_sys_tray_icon = new QSystemTrayIcon(this);
  m_tray_menu = new QMenu(this);
  m_sys_tray_icon->setContextMenu(m_tray_menu);

  m_tray_menu->addAction(m_act_info);
  m_tray_menu->addAction(m_act_generate_ssh);
  m_tray_menu->addSeparator();

  m_launch_menu = m_tray_menu->addMenu(QIcon(":/hub/Launch-07.png"),
                                       tr("Launch"));
  m_hub_menu = m_tray_menu->addMenu(QIcon(":/hub/Environmetns-07.png"),
                                    tr("Environments"));

#ifdef RT_OS_WINDOWS
  m_vbox_menu = m_tray_menu->addMenu(tr("Virtual machines"));
#else
  m_vbox_menu = new QMenu(this);
  m_tray_menu->addAction(m_act_vbox);
#endif

  m_vbox_menu->setIcon(QIcon(":/hub/VM-07.png"));

  fill_vm_menu();
  fill_launch_menu();

  m_vboxAction = new QWidgetAction(m_vbox_menu);
  m_vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(m_vboxAction);  

  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_settings);
  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_logout);
  m_tray_menu->addAction(m_act_notifications_history);
  m_tray_menu->addAction(m_act_about);
  m_tray_menu->addAction(m_act_quit);

  m_sys_tray_icon->setIcon(QIcon(":/hub/Tray_icon_set-07.png"));
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::notification_received(CNotificationObserver::notification_level_t level,
                                         const QString &msg) {
  static const QString titles[] = {"Info", "Warning", "Error", "Critical"};

  //3rd element is warning, because critical shows messagebox when we need only notification.
  //we will show message box when critical errors appears
  static const QSystemTrayIcon::MessageIcon icons[] = {
    QSystemTrayIcon::Information, QSystemTrayIcon::Warning,
    QSystemTrayIcon::Warning, QSystemTrayIcon::Critical
  };

  if (CSettingsManager::Instance().is_notification_ignored(msg)) {
    //do we need som logs here ?
    return;
  }

  if (QSystemTrayIcon::supportsMessages()) {
    m_sys_tray_icon->showMessage(titles[level],
                                 msg,
                                 icons[level],
                                 CSettingsManager::Instance().notification_delay_sec() * 1000); //todo add delay to settings
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::logout() {
  CHubController::Instance().suspend();
  this->m_sys_tray_icon->hide();

  DlgLogin dlg;
  connect(&dlg, &DlgLogin::login_success, this, &TrayControlWindow::login_success);
  dlg.setModal(true);
  if (dlg.exec() != QDialog::Accepted) {
    qApp->exit(0);
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::login_success() {
  CHubController::Instance().start();
  this->m_sys_tray_icon->show();
}
////////////////////////////////////////////////////////////////////////////

/*** Vbox slots  ***/
void
TrayControlWindow::vm_added(const QString &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(m_vboxAction);

  add_vm_menu(vm_id);
  m_vboxAction = new QWidgetAction(m_vbox_menu);
  m_vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(m_vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_removed(const QString &vm_id) {
  m_vbox_menu->hide();
  m_vbox_menu->removeAction(m_vboxAction);
  remove_vm_menu(vm_id);
  m_vboxAction = new QWidgetAction(m_vbox_menu);
  m_vboxAction->setDefaultWidget(m_w_Player);
  m_vbox_menu->addAction(m_vboxAction);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_state_changed(const QString &vm_id) {
  auto ip = m_dct_player_menus.find(vm_id);
  if (ip == m_dct_player_menus.end()) return;
  const CVirtualMachine *vm = CVboxManager::Instance()->vm_by_id(vm_id);
  if (vm == NULL) {
    //todo log
    return;
  }
  MachineState_T ns = vm->state();
  ip->second->set_buttons(ns);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vm_session_state_changed(const QString &vm_id) {
  UNUSED_ARG(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vmc_player_act_released(const QString &vm_id) { // remove
  const CVirtualMachine *vm = CVboxManager::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  bool on = vm->state() == MachineState_PoweredOff;

  if (on) {
    CVboxManager::Instance()->pause(vm_id);
    return;
  } //turn on
}
////////////////////////////////////////////////////////////////////////////


void
TrayControlWindow::hub_container_mi_triggered(const CEnvironment *env,
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
TrayControlWindow::update_available(QString file_id) {
  CNotificationObserver::Info(
        QString("Update for %1 is available. Check \"About\" dialog").arg(file_id));
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::update_finished(QString file_id,
                                   bool success) {
  if (!success) {
    CNotificationObserver::Error(
          QString("Failed to update %1. See details in error logs").arg(file_id));
    return;
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::launch_Hub() {
  QString browser = "/etc/alternatives/x-www-browser"; //default browser
  QString folder;
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
  args << hub_site();

  if (!QProcess::startDetached(browser, args, folder)) {
    QString err_msg = QString("Launch hub website failed");
    CNotificationObserver::Error(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    return;
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::launch_ss_triggered() {
  QAction* act = qobject_cast<QAction*>(sender());
  act->setEnabled(false);
  launch_ss();
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::environments_updated_sl(int rr) {
  UNUSED_ARG(rr);
  static std::vector<QString> lst_checked_unhealthy_env;
  m_hub_menu->clear();
  for (auto i = m_lst_hub_menu_items.begin(); i != m_lst_hub_menu_items.end(); ++i) {
    delete *i;
  }

  m_lst_hub_menu_items.clear();
  std::vector<QString> lst_unhealthy_envs;
  std::vector<QString> lst_unhealthy_env_statuses;

  if (CHubController::Instance().lst_environments().empty()) {
    QAction* empty_action = new QAction("Empty", this);
    empty_action->setEnabled(false);
    m_hub_menu->addAction(empty_action);
    return;
  }

  for (auto env = CHubController::Instance().lst_environments().cbegin();
       env != CHubController::Instance().lst_environments().cend(); ++env) {

    QString env_name = env->name();
#ifdef RT_OS_LINUX
    env_name.replace("_", "__"); //megahack :) Don't know how to handle underscores.
#endif
    QMenu* env_menu = m_hub_menu->addMenu(env_name);

    std::vector<QString>::iterator iter_found =
        std::find(lst_checked_unhealthy_env.begin(),
                  lst_checked_unhealthy_env.end(), env->id());

    if (!env->healthy()) {
      if (iter_found == lst_checked_unhealthy_env.end()) {
        lst_unhealthy_envs.push_back(env_name);
        lst_unhealthy_env_statuses.push_back(env->status());
        lst_checked_unhealthy_env.push_back(env->id());
        CApplicationLog::Instance()->LogError("Environment %s, %s is unhealthy. Reason : %s",
                                              env_name.toStdString().c_str(),
                                              env->id().toStdString().c_str(),
                                              env->status_description().toStdString().c_str());
      }
    } else {
      if (iter_found != lst_checked_unhealthy_env.end()) {
        CNotificationObserver::Info(QString("Environment %1 became healthy").arg(env->name()));
        CApplicationLog::Instance()->LogTrace("Environment %s became healthy", env->name().toStdString().c_str());
        lst_checked_unhealthy_env.erase(iter_found);
      }
    }

    for (auto cont = env->containers().cbegin(); cont != env->containers().cend(); ++cont) {
      QString cont_name = cont->name();
#ifdef RT_OS_LINUX
      cont_name.replace("_", "__"); //megahack :) Don't know how to handle underscores.
#endif
      QAction* act = new QAction(cont_name, this);
      act->setEnabled(env->healthy() && !cont->rh_ip().isNull() && !cont->rh_ip().isEmpty());

      CHubEnvironmentMenuItem* item =
          new CHubEnvironmentMenuItem(&(*env), &(*cont), m_sys_tray_icon);
      connect(act, &QAction::triggered, item,
              &CHubEnvironmentMenuItem::internal_action_triggered);
      connect(item, &CHubEnvironmentMenuItem::action_triggered,
              this, &TrayControlWindow::hub_container_mi_triggered);
      env_menu->addAction(act);
      m_lst_hub_menu_items.push_back(item);
    }
  }

  if (lst_unhealthy_envs.empty()) return;

  QString str_unhealthy_envs = "";
  QString str_statuses = "";
  for (size_t i = 0; i < lst_unhealthy_envs.size()-1; ++i) {
    str_unhealthy_envs += lst_unhealthy_envs[i] + ", ";
    str_statuses += lst_unhealthy_env_statuses[i] + ", ";
  }

  str_unhealthy_envs += lst_unhealthy_envs[lst_unhealthy_envs.size()-1];
  str_statuses += lst_unhealthy_env_statuses[lst_unhealthy_envs.size()-1];

  QString str_notification = QString("Environment%1 %2 %3 %4").
                             arg(lst_unhealthy_envs.size() > 1 ? "s" : "").
                             arg(str_unhealthy_envs).
                             arg(lst_unhealthy_envs.size() > 1 ? "are" : "is").
                             arg(str_statuses);

  CNotificationObserver::Instance()->Info(str_notification);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::balance_updated_sl() {
  m_act_info->setText(CHubController::Instance().balance());
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::got_ss_console_readiness_sl(bool is_ready,
                                               QString err) {
  if (!is_ready) {
    CNotificationObserver::Info(err);
    m_act_launch_SS->setEnabled(true);
    return;
  }

  QString hub_url = "https://localhost:9999";

  std::string rh_ip;
  int ec = 0;

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::get_rh_ip_via_libssh2(
        CSettingsManager::Instance().rh_host().toStdString().c_str(),
        CSettingsManager::Instance().rh_port(),
        CSettingsManager::Instance().rh_user().toStdString().c_str(),
        CSettingsManager::Instance().rh_pass().toStdString().c_str(),
        ec,
        rh_ip);

  if (scwe == SCWE_SUCCESS && (ec == RLE_SUCCESS || ec == 0)) {
    hub_url = QString("https://%1:8443").arg(rh_ip.c_str());
  } else {
    CApplicationLog::Instance()->LogError("Can't get RH IP address. Err : %s, exit_code : %d",
                                          CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)scwe), ec);
    CNotificationObserver::Info(QString("Can't get RH IP address. Error : %1, Exit_Code : %2").
                                arg(CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)scwe)).
                                arg(ec));
    m_act_launch_SS->setEnabled(true);
    return;
  }

  QString browser; // "/etc/alternatives/x-www-browser";
  QString folder;
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
  if (!QProcess::startDetached(browser, args, folder)) {
    QString err_msg = QString("Run SS console failed. Can't start process");
    CNotificationObserver::Error(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    m_act_launch_SS->setEnabled(true);
    return;
  }
  m_act_launch_SS->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_play_triggered(const QString& vm_id) {
  int32_t rc;
  const CVirtualMachine *vm = CVboxManager::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  MachineState_T state = vm->state();
  if (state < 5) { //Powered off
    rc = CVboxManager::Instance()->launch_vm(vm_id);
    return;
  }

  if (state == MachineState_Running) {
    rc = CVboxManager::Instance()->pause(vm_id);
    return;
  }

  if (state == MachineState_Paused ||
      state == MachineState_Teleporting ||
      state == MachineState_LiveSnapshotting) {
    rc = CVboxManager::Instance()->resume(vm_id);
    return;
  }
  //todo use rc
  (void)rc;
  return;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_stop_triggered(const QString& vm_id) {
  int32_t rc;
  const CVirtualMachine *vm = CVboxManager::Instance()->vm_by_id(vm_id);
  if (vm == NULL)
    return;
  MachineState_T state = vm->state();
  if (state < 5) {
    return;
  }

  rc = CVboxManager::Instance()->poweroff(vm_id);
  //todo check rc
  (void)rc;
  return;
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_add_triggered(const QString& vm_id) {
  //todo check result
  CVboxManager::Instance()->add(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::vbox_menu_btn_rem_triggered(const QString& vm_id) {
  //todo check result
  CVboxManager::Instance()->remove(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::launch_ss() {
  std::string rh_ip;
  int ec = 0;

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::get_rh_ip_via_libssh2(
        CSettingsManager::Instance().rh_host().toStdString().c_str(),
        CSettingsManager::Instance().rh_port(),
        CSettingsManager::Instance().rh_user().toStdString().c_str(),
        CSettingsManager::Instance().rh_pass().toStdString().c_str(),
        ec,
        rh_ip);

  if (scwe == SCWE_SUCCESS && (ec == RLE_SUCCESS || ec == 0)) {
    //after that got_ss_console_readiness_sl will be called
    CRestWorker::Instance()->check_if_ss_console_is_ready(
          QString("https://%1:8443/rest/v1/peer/ready").arg(rh_ip.c_str()));
  } else {
    CApplicationLog::Instance()->LogError("Can't get RH IP address. Err : %s, exit_code : %d",
                                          CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)scwe), ec);
    CNotificationObserver::Info(QString("Can't get RH IP address. Error : %1, Exit_Code : %2").
                                arg(CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)scwe)).
                                arg(ec));
    m_act_launch_SS->setEnabled(true);
    return;
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::show_dialog(QDialog* (*pf_dlg_create)(QWidget*), const QString& title) {
  std::map<QString, QDialog*>::iterator iter =
      m_dct_active_dialogs.find(title);

  if (iter == m_dct_active_dialogs.end()) {
    QDialog* dlg = pf_dlg_create(this);
    dlg->setWindowTitle(title);
    m_dct_active_dialogs[dlg->windowTitle()] = dlg;

    int dst_x, dst_y, src_x, src_y, icon_x, icon_y;
    dst_x = dst_y = 0;
    src_x = src_y = 0;

    icon_x = m_sys_tray_icon->geometry().x();
    icon_y = m_sys_tray_icon->geometry().y();

    qDebug() << m_sys_tray_icon->geometry();
    if (icon_x == 0 && icon_y == 0) {
      icon_x = QCursor::pos().x();
      icon_y = QCursor::pos().y();
    }

    int sw, sh;
    sw = QApplication::primaryScreen()->geometry().width();
    sh = QApplication::primaryScreen()->geometry().height();

    src_x = icon_x < sw/2 ? -dlg->width() : sw+dlg->width();
    src_y = icon_y < sh/2 ? 0 : sh-dlg->height();
    dst_x = icon_x < sw/2 ? 0 : sw-dlg->width();
    dst_y = src_y;

    if (CSettingsManager::Instance().use_animations()) {
      QPropertyAnimation *pos_anim = new QPropertyAnimation(dlg, "pos");
      QPropertyAnimation *opa_anim = new QPropertyAnimation(dlg, "windowOpacity");

      pos_anim->setStartValue(QPoint(src_x, src_y));
      pos_anim->setEndValue(QPoint(dst_x, dst_y));
      pos_anim->setEasingCurve(QEasingCurve::OutBack);
      pos_anim->setDuration(800);

      opa_anim->setStartValue(0.0);
      opa_anim->setEndValue(1.0);
      opa_anim->setEasingCurve(QEasingCurve::Linear);
      opa_anim->setDuration(800);

      QParallelAnimationGroup *gr = new QParallelAnimationGroup;
      gr->addAnimation(pos_anim);
      gr->addAnimation(opa_anim);

      dlg->show();
      gr->start();
      connect(gr, &QParallelAnimationGroup::finished, [dlg]() {
        dlg->activateWindow();
        dlg->raise();
        dlg->setFocus();
      });
      connect(gr, &QParallelAnimationGroup::finished, gr, &QParallelAnimationGroup::deleteLater);
    } else {
      dlg->move(dst_x, dst_y);
      dlg->show();
      dlg->activateWindow();
      dlg->raise();
      dlg->setFocus();
    }
    connect(dlg, &QDialog::finished, this, &TrayControlWindow::dialog_closed);
  } else {
    if (iter->second) {
      iter->second->show();
      iter->second->activateWindow();
      iter->second->raise();
      iter->second->setFocus();
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::dialog_closed(int unused) {
  UNUSED_ARG(unused);
  QDialog* dlg = qobject_cast<QDialog*>(sender());
  if (dlg == nullptr) return;
  QString title = dlg->windowTitle();
  dlg->deleteLater();
  auto iter = m_dct_active_dialogs.find(title);
  if (iter == m_dct_active_dialogs.end()) return;
  m_dct_active_dialogs.erase(iter);
}
////////////////////////////////////////////////////////////////////////////

QDialog* create_settings_dialog(QWidget* p) {return new DlgSettings(p);}
void
TrayControlWindow::show_settings_dialog() {
  show_dialog(create_settings_dialog, "Settings");
}
////////////////////////////////////////////////////////////////////////////

QDialog* create_about_dialog(QWidget* p) {return new DlgAbout(p);}
void
TrayControlWindow::show_about() {
  show_dialog(create_about_dialog, "About Subutai Tray");
}
////////////////////////////////////////////////////////////////////////////

QDialog* create_ssh_key_generate_dialog(QWidget* p) {return new DlgGenerateSshKey(p);}
void
TrayControlWindow::ssh_key_generate_triggered() {
  show_dialog(create_ssh_key_generate_dialog, "SSH key generation");
}

QDialog* create_notifications_dialog(QWidget* p) {return new DlgNotifications(p);}
void
TrayControlWindow::show_notifications_triggered() {
  show_dialog(create_notifications_dialog, "Notifications history");
}
////////////////////////////////////////////////////////////////////////////
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
  m_vm_count(0) {
  UNUSED_ARG(parent);
  m_lblHeader = new QLabel(this);
  m_lblHeader->setText("No resource hosts registered");
  m_lblHeader->setMinimumWidth(180);
  m_lblHeader->setSizePolicy(QSizePolicy::Preferred,
                             QSizePolicy::Minimum);
  m_vLayout = new QVBoxLayout(0);
  m_vLayout->setSpacing(2);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  m_vLayout->addWidget(m_lblHeader);
  this->setLayout(m_vLayout);
}

CVBPlayer::~CVBPlayer(){
}
////////////////////////////////////////////////////////////////////////////

void
CVBPlayer::add(CVBPlayerItem* pItem) {
  if (m_vm_count == 0){
    m_lblHeader->setText("Resource hosts registered:");
    m_lblHeader->setVisible(false);
  }
  m_vLayout->addWidget(pItem);
  int cnt = m_vLayout->layout()->count();

  this->setMinimumHeight(30*(cnt+1));
  this->setMaximumHeight(30*(cnt+1));

  m_vm_count++;
  this->setLayout(m_vLayout);
  this->setVisible(true);
}
////////////////////////////////////////////////////////////////////////////

void
CVBPlayer::remove(CVBPlayerItem* pItem) {
  m_vLayout->removeWidget(pItem);
  int cnt = m_vLayout->layout()->count();

  this->setMinimumHeight(30*(cnt+1));
  this->setMaximumHeight(30*(cnt+1));
  m_vm_count--;
  if (m_vm_count == 0){
    m_lblHeader->setText("No resource hosts registered:");
    m_lblHeader->setVisible(true);
  }
  this->setLayout(m_vLayout);
  this->setVisible(true);
}
/////////////////////////////////////////////////////////////////////////////

CVBPlayerItem::CVBPlayerItem(const CVirtualMachine *vm, QWidget* parent) :
  m_vm_player_item_id(vm->id()) {

  UNUSED_ARG(parent);
  m_lbl_name = new QLabel(this);
  m_lbl_state = new QLabel(this);
  m_btn_play = new QPushButton("", this);
  m_btn_stop = new QPushButton("", this);

  p_h_Layout = new QHBoxLayout(NULL);
  m_lbl_name->setMinimumWidth(180);
  m_lbl_state->setMinimumWidth(100);
  m_lbl_state->setMaximumWidth(100);

  m_lbl_name->setText(vm->name());
  m_lbl_state->setText(CVirtualMachine::vm_state_to_str(vm->state()));

  m_btn_play->setIcon(QIcon(":/hub/Launch-07.png"));
  m_btn_stop->setIcon(QIcon(":/hub/Stop-07.png"));

  m_btn_play->setToolTip("Play/Pause/Resume");
  m_btn_stop->setToolTip("Power off");
  connect(m_btn_play, &QPushButton::released,
          this, &CVBPlayerItem::vbox_menu_btn_play_released, Qt::QueuedConnection);
  connect(m_btn_stop, &QPushButton::released,
          this, &CVBPlayerItem::vbox_menu_btn_stop_released, Qt::QueuedConnection);

  set_buttons(vm->state());
  p_h_Layout->addWidget(m_lbl_name);
  p_h_Layout->addWidget(m_lbl_state);

  p_h_Layout->addWidget(m_btn_play);
  p_h_Layout->addWidget(m_btn_stop);

  p_h_Layout->setMargin(1);
  p_h_Layout->setSpacing(2);
  p_h_Layout->setObjectName(vm->name());
  this->setLayout(p_h_Layout);
}
////////////////////////////////////////////////////////////////////////////

CVBPlayerItem::~CVBPlayerItem(){
  p_h_Layout->removeWidget(m_lbl_name);
  p_h_Layout->removeWidget(m_lbl_state);
  p_h_Layout->removeWidget(m_btn_play);
  p_h_Layout->removeWidget(m_btn_stop);
}
////////////////////////////////////////////////////////////////////////////

void
CVBPlayerItem::set_buttons(MachineState_T state) {
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

  m_lbl_state->setText(CVirtualMachine::vm_state_to_str(state));
  int isi = 0;
  if (state < 5) isi = 0;
  else if (state == MachineState_Running) isi = 1;
  else if (state == MachineState_Paused) isi = 2;
  else if (state == MachineState_Stuck) isi = 3;
  else if (state == MachineState_Teleporting ||
           state == MachineState_LiveSnapshotting) isi = 4;
  else isi = 5; //state >= 10

  m_btn_play->setIcon(icon_set[isi].play);
  m_btn_stop->setIcon(icon_set[isi].stop);
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
///////////////////////////////////////////////////////////////////////////

void
TrayControlWindow::ssh_to_container_finished(int result,
                                             void *additional_data) {
  if (result != SLE_SUCCESS) {
    CNotificationObserver::Error(
          QString("Can't ssh to container. Err : %1").arg(CHubController::ssh_launch_err_to_str(result)));
  }
  QAction* act = static_cast<QAction*>(additional_data);
  if (act == NULL) return;
  act->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////
