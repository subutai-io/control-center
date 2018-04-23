#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QWidgetAction>
#include <QtConcurrent/QtConcurrent>
#include <QtGui>
#include <algorithm>

#include "DlgAbout.h"
#include "DlgGenerateSshKey.h"
#include "DlgCreatePeer.h"
#include "DlgLogin.h"
#include "DlgNotification.h"
#include "DlgNotifications.h"
#include "DlgSettings.h"
#include "DlgEnvironment.h"
#include "DlgPeer.h"
#include "HubController.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"
#include "libssh2/include/LibsshController.h"
#include "ui_TrayControlWindow.h"
#include "updater/HubComponentsUpdater.h"
#include "DlgEnvironment.h"
#include "P2PController.h"
#include "RhController.h"
#include "PeerController.h"

using namespace update_system;

template<class OS> static inline void
InitTrayIconTriggerHandler_internal(QSystemTrayIcon* icon,
                                    TrayControlWindow* win);

template<>
inline void InitTrayIconTriggerHandler_internal<Os2Type<OS_WIN> >(
        QSystemTrayIcon *icon, TrayControlWindow *win) {
    QObject::connect(icon, &QSystemTrayIcon::activated,
                     win, &TrayControlWindow::tray_icon_is_activated_sl);
}

template<>
inline void InitTrayIconTriggerHandler_internal<Os2Type<OS_LINUX> >(
        QSystemTrayIcon *icon, TrayControlWindow *win) {
    UNUSED_ARG(icon);
    UNUSED_ARG(win);
}

template<>
inline void InitTrayIconTriggerHandler_internal<Os2Type<OS_MAC> >(
        QSystemTrayIcon *icon, TrayControlWindow *win) {
    UNUSED_ARG(icon);
    UNUSED_ARG(win);
}

void InitTrayIconTriggerHandler(QSystemTrayIcon *icon,
                                TrayControlWindow *win) {
  InitTrayIconTriggerHandler_internal<Os2Type<CURRENT_OS> >(icon, win);
}


TrayControlWindow::TrayControlWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::TrayControlWindow),
      m_act_ssh_keys_management(NULL),
      m_act_quit(NULL),
      m_act_settings(NULL),
      m_act_balance(NULL),
      m_act_hub(NULL),
      m_act_launch_Hub(NULL),
      m_act_about(NULL),
      m_act_logout(NULL),
      m_sys_tray_icon(NULL),
      m_act_create_peer(NULL),
      m_tray_menu(NULL),
      m_act_p2p_status(NULL),
      in_peer_slot(false){

  ui->setupUi(this);

  create_tray_actions();
  create_tray_icon();
  m_sys_tray_icon->show();
  p2p_current_status = P2PStatus_checker::P2P_LOADING;

  CPeerController::Instance()->init();

  connect(CNotificationObserver::Instance(), &CNotificationObserver::notify,
          this, &TrayControlWindow::notification_received);

  connect(&CHubController::Instance(),
          &CHubController::ssh_to_container_from_tray_finished,
          this, &TrayControlWindow::ssh_to_container_finished);

  connect(&CHubController::Instance(),
          &CHubController::desktop_to_container_from_tray_finished, this,
          &TrayControlWindow::desktop_to_container_finished);

  connect(&CHubController::Instance(), &CHubController::balance_updated, this,
          &TrayControlWindow::balance_updated_sl);
  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &TrayControlWindow::environments_updated_sl);
  connect(&CHubController::Instance(), &CHubController::my_peers_updated,
          this, &TrayControlWindow::my_peers_updated_sl);

  connect(CRestWorker::Instance(), &CRestWorker::on_got_ss_console_readiness,
          this, &TrayControlWindow::got_ss_console_readiness_sl);

  connect(CHubComponentsUpdater::Instance(),
          &CHubComponentsUpdater::updating_finished, this,
          &TrayControlWindow::update_finished);
  connect(CHubComponentsUpdater::Instance(),
          &CHubComponentsUpdater::update_available, this,
          &TrayControlWindow::update_available);
  connect(CRhController::Instance(), &CRhController::ssh_to_rh_finished, this,
          &TrayControlWindow::ssh_to_rh_finished_sl);

  connect(CPeerController::Instance(), &CPeerController::got_peer_info,
          this, &TrayControlWindow::got_peer_info_sl);

  /*p2p status updater*/
  connect(&P2PStatus_checker::Instance(), &P2PStatus_checker::p2p_status, this,
          &TrayControlWindow::update_p2p_status_sl);

  InitTrayIconTriggerHandler(m_sys_tray_icon, this);
  CHubController::Instance().force_refresh();
  login_success();
}

TrayControlWindow::~TrayControlWindow() {
  QMenu* menus[] = {m_hub_menu, m_hub_peer_menu, m_local_peer_menu, m_tray_menu};
  QAction* acts[] = {m_act_ssh_keys_management,
                     m_act_quit,
                     m_act_settings,
                     m_act_balance,
                     m_act_hub,
                     m_act_launch_Hub,
                     m_act_about,
                     m_act_logout,
                     m_act_notifications_history,
                     m_act_p2p_status,
                     m_act_create_peer};

  for (size_t i = 0; i < sizeof(menus) / sizeof(QMenu*); ++i) {
    if (menus[i] == nullptr) continue;
    try {
      delete menus[i];
    } catch (...) { /*do nothing*/
    }
  }

  for (size_t i = 0; i < sizeof(acts) / sizeof(QAction*); ++i) {
    if (acts[i] == nullptr) continue;
    try {
      delete acts[i];
    } catch (...) { /*do nothing*/
    }
  }

  try {
    delete m_sys_tray_icon;
  } catch (...) {
  }
  delete ui;
}
////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::application_quit() {
  qDebug() << "Quitting the tray";
  m_sys_tray_icon->hide();
  QApplication::quit();
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::create_tray_actions() {

  m_act_settings =
      new QAction(QIcon(":/hub/Settings-07.png"), tr("Settings"), this);
  connect(m_act_settings, &QAction::triggered, this,
          &TrayControlWindow::show_settings_dialog);

  m_act_hub =
      new QAction(QIcon(":/hub/Environmetns-07.png"), tr("Environments"), this);

  m_act_quit = new QAction(QIcon(":/hub/Exit-07"), tr("Quit"), this);
  connect(m_act_quit, &QAction::triggered, this,
          &TrayControlWindow::application_quit);

  m_act_launch_Hub =
      new QAction(QIcon(":/hub/Hub-07.png"), tr("Go to Bazaar"), this);
  connect(m_act_launch_Hub, &QAction::triggered, this,
          &TrayControlWindow::launch_Hub);

  m_act_balance = new QAction(QIcon(":/hub/Balance-07.png"),
                              CHubController::Instance().balance(), this);
  connect(m_act_balance, &QAction::triggered,
          [] { CHubController::Instance().launch_balance_page(); });

  m_act_about = new QAction(QIcon(":/hub/about.png"), tr("About"), this);
  connect(m_act_about, &QAction::triggered, this,
          &TrayControlWindow::show_about);

  m_act_ssh_keys_management =
      new QAction(QIcon(":/hub/ssh-keys.png"), tr("SSH-keys management"), this);
  connect(m_act_ssh_keys_management, &QAction::triggered, this,
          &TrayControlWindow::ssh_key_generate_triggered);

  m_act_logout = new QAction(QIcon(":/hub/logout.png"), tr("Logout"), this);
  connect(m_act_logout, &QAction::triggered, this, &TrayControlWindow::logout);

  m_act_notifications_history = new QAction(
      QIcon(":hub/notifications_history.png"), tr("Notifications history"), this);
  connect(m_act_notifications_history, &QAction::triggered, this,
          &TrayControlWindow::show_notifications_triggered);

  /*p2p status*/
  m_act_p2p_status = new QAction(
        QIcon(":hub/loading.png"), tr("P2P is loading..."), this);
  connect(m_act_p2p_status, &QAction::triggered, this,
          &TrayControlWindow::launch_p2p);

  m_act_create_peer = new QAction(QIcon(":hub/add.png"), tr("Create peer"), this);
  connect(m_act_create_peer, &QAction::triggered, this,
          &TrayControlWindow::show_create_dialog);
  m_empty_action = new QAction(tr("Empty"), this);
  m_empty_action->setEnabled(false);
}
////////////////////////////////////////////////////////////////////////////


void TrayControlWindow::create_tray_icon() {
  m_sys_tray_icon = new QSystemTrayIcon(this);
  m_tray_menu = new QMenu(this);
  m_sys_tray_icon->setContextMenu(m_tray_menu);
  /*p2p status icon*/
  m_tray_menu->addAction(m_act_p2p_status);
  m_tray_menu->addSeparator();

  m_tray_menu->addAction(m_act_launch_Hub);
  m_tray_menu->addAction(m_act_balance);

  m_tray_menu->addSeparator();

  m_hub_menu = m_tray_menu->addMenu(QIcon(":/hub/Environmetns-07.png"),
                                    tr("Environments"));
  m_hub_menu->setStyleSheet(qApp->styleSheet());
  m_hub_peer_menu = m_tray_menu->addMenu(QIcon(":/hub/tray.png"),
                                     tr("My Peers"));
  m_hub_peer_menu->addAction(m_empty_action);
  m_local_peer_menu = m_tray_menu->addMenu(QIcon(":/hub/Launch-07.png"),
                                     tr("Local Peers"));
  m_local_peer_menu->addAction(m_empty_action);
  m_tray_menu->addAction(m_act_create_peer);
  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_settings);
  m_tray_menu->addAction(m_act_ssh_keys_management);
  m_tray_menu->addAction(m_act_notifications_history);
  m_tray_menu->addSeparator();
  m_tray_menu->addAction(m_act_about);
  m_tray_menu->addAction(m_act_logout);
  m_tray_menu->addAction(m_act_quit);

  m_sys_tray_icon->setIcon(QIcon(":/hub/cc_icon.png"));
}

void TrayControlWindow::get_sys_tray_icon_coordinates_for_dialog(
    int& src_x, int& src_y, int& dst_x, int& dst_y, int dlg_w, int dlg_h,
    bool use_cursor_position) {
  int icon_x, icon_y;
  dst_x = dst_y = 0;
  src_x = src_y = 0;

  icon_x = m_sys_tray_icon->geometry().x();
  icon_y = m_sys_tray_icon->geometry().y();

  int adw, adh;
  adw = QApplication::desktop()->availableGeometry().width();
  adh = QApplication::desktop()->availableGeometry().height();

  if (icon_x == 0 && icon_y == 0) {
    if (use_cursor_position) {
      icon_x = QCursor::pos().x();
      icon_y = QCursor::pos().y();
    } else {
      int coords[] = {adw, 0, adw, adh, 0, adh, 0, 0};
      uint32_t pc =
          CSettingsManager::Instance().preferred_notifications_place();
      icon_x = coords[pc * 2];
      icon_y = coords[pc * 2 + 1];
    }
  }

  int dx, dy;
  dy = QApplication::desktop()->availableGeometry().y();
  dx = QApplication::desktop()->availableGeometry().x();

#ifdef RT_OS_WINDOWS
  dy += dy ? 0 : 35;  // don't know why -20 and 35
#endif

  if (icon_x < adw / 2) {
    src_x = -dlg_w + dx;
    dst_x = src_x + dlg_w;
  } else {
    src_x = adw + dx;
    dst_x = src_x - dlg_w;
  }

  src_y = icon_y < adh / 2 ? dy : adh - dy - dlg_h;
  dst_y = src_y;
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::tray_icon_is_activated_sl(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger) {
    m_sys_tray_icon->contextMenu()->exec(QPoint(QCursor::pos().x() ,QCursor::pos().y()));
  }
}

////////////////////////////////////////////////////////////////////////////

static QPoint lastNotificationPos(0, 0);
template<class OS>
static inline void shift_notification_dialog_positions_internal(int &src_y , int &dst_y, int shift_value);

template <>
inline void shift_notification_dialog_positions_internal< Os2Type<OS_LINUX> >(int &src_y , int &dst_y, int shift_value){
  const int &pref_place = CSettingsManager::Instance().preferred_notifications_place();
  if (pref_place == 1 || pref_place == 2) { // if the notification dialogs on the top of the screen
    src_y = lastNotificationPos.y() - shift_value;
    dst_y = lastNotificationPos.y() - shift_value;
  }
  else { // on the bottom
    src_y = lastNotificationPos.y() + shift_value;
    dst_y = lastNotificationPos.y() + shift_value;
  }
}
template<>
inline void shift_notification_dialog_positions_internal< Os2Type<OS_WIN> >(int &src_y , int &dst_y, int shift_value){
  src_y = lastNotificationPos.y() - shift_value;
  dst_y = lastNotificationPos.y() - shift_value;
}

template<>
inline void shift_notification_dialog_positions_internal< Os2Type<OS_MAC> >(int &src_y , int &dst_y, int shift_value){
  src_y = lastNotificationPos.y() + shift_value;
  dst_y = lastNotificationPos.y() + shift_value;
}

void shift_notification_dialog_positions(int &src_y, int &dst_y, int shift_value) {
  shift_notification_dialog_positions_internal< Os2Type<CURRENT_OS> >(src_y, dst_y, shift_value);
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::notification_received(
    CNotificationObserver::notification_level_t level, const QString& msg,
    DlgNotification::NOTIFICATION_ACTION_TYPE action_type) {
  qDebug()
      << "Message: " << msg
      << "Level: " << CNotificationObserver::notification_level_to_str(level)
      << "Action Type: " << (size_t)action_type
      << "Current notification level: " << CSettingsManager::Instance().notifications_level()
      << "Message is ignored: " << CSettingsManager::Instance().is_notification_ignored(msg);

  static std::map<DlgNotification::NOTIFICATION_ACTION_TYPE, bool>  no_ignore = {std::make_pair(DlgNotification::N_START_P2P, 1),
                                                                                std::make_pair(DlgNotification::N_INSTALL_P2P, 1),
                                                                                std::make_pair(DlgNotification::N_STOP_P2P, 1),
                                                                                std::make_pair(DlgNotification::N_ABOUT, 1),
                                                                                std::make_pair(DlgNotification::N_SETTINGS, 1)
                                                                                };

  if (CSettingsManager::Instance().is_notification_ignored(msg)||
      (uint32_t)level < CSettingsManager::Instance().notifications_level()) {
      if(no_ignore[action_type] != 1)
        return;
  }

  QDialog* dlg = new DlgNotification(level, msg, this, action_type);

  dlg->setWindowFlags(dlg->windowFlags() | Qt::WindowStaysOnTopHint);

  connect(dlg, &QDialog::finished, dlg, &DlgNotification::deleteLater);
  int src_x, src_y, dst_x, dst_y;
  get_sys_tray_icon_coordinates_for_dialog(src_x, src_y, dst_x, dst_y,
                                           dlg->width(), dlg->height(), false);
  if (DlgNotification::NOTIFICATIONS_COUNT > 1 && DlgNotification::NOTIFICATIONS_COUNT < 4) { // shift dialog if there is more than one dialogs
      shift_notification_dialog_positions(src_y, dst_y, dlg->height() + 20);
  }

  if (CSettingsManager::Instance().use_animations()) {
    QPropertyAnimation* pos_anim = new QPropertyAnimation(dlg, "pos");
    QPropertyAnimation* opa_anim = new QPropertyAnimation(dlg, "windowOpacity");

    pos_anim->setStartValue(QPoint(src_x, src_y));
    pos_anim->setEndValue(QPoint(dst_x, dst_y));
    pos_anim->setEasingCurve(QEasingCurve::OutBack);
    pos_anim->setDuration(800);

    opa_anim->setStartValue(0.0);
    opa_anim->setEndValue(1.0);
    opa_anim->setEasingCurve(QEasingCurve::Linear);
    opa_anim->setDuration(800);

    QParallelAnimationGroup* gr = new QParallelAnimationGroup;
    gr->addAnimation(pos_anim);
    gr->addAnimation(opa_anim);

    dlg->move(src_x, src_y);

    dlg->show();
    gr->start();
    connect(gr, &QParallelAnimationGroup::finished, gr,
            &QParallelAnimationGroup::deleteLater);
  } else {
    dlg->move(dst_x, dst_y);
    dlg->show();
  }
  lastNotificationPos = dlg->pos();
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::logout() {
  std::vector<QDialog*> lstActiveDialogs(m_dct_active_dialogs.size());
  int i = 0;
  //this extra copy because on dialog finish we are removing it from m_dct_active_dialogs
  for (auto j = m_dct_active_dialogs.begin(); j != m_dct_active_dialogs.end(); ++j, ++i)
    lstActiveDialogs[i] = j->second;

  //close active dialogs
  while(i--)
    lstActiveDialogs[i]->close();

  CHubController::Instance().logout();
  this->m_sys_tray_icon->hide();
  CSettingsManager::Instance().set_remember_me(false);
  DlgLogin dlg;
  connect(&dlg, &DlgLogin::login_success, this,
          &TrayControlWindow::login_success);
  dlg.setModal(true);
  if (dlg.exec() != QDialog::Accepted) {
    qApp->exit(0);
  }
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::login_success() {
  CHubController::Instance().force_refresh();
  m_sys_tray_icon->show();
}

////////////////////////////////////////////////////////////////////////////
#include "DlgTransferFile.h"

void TrayControlWindow::upload_to_container_triggered(const CEnvironment &env,
                                                      const CHubContainer &cont) {
  //generate_transferfile_dlg();
  DlgTransferFile *dlg_transfer_file = new DlgTransferFile(this);
  Qt::WindowFlags flags = 0;
  flags = Qt::Window;
  flags |= Qt::WindowMinimizeButtonHint;
  flags |= Qt::WindowMaximizeButtonHint;
  flags |= Qt::WindowCloseButtonHint;
  dlg_transfer_file->setWindowFlags(flags);
  CSystemCallWrapper::container_ip_and_port cip =
      CSystemCallWrapper::container_ip_from_ifconfig_analog(cont.port(), cont.ip(), cont.rh_ip());

  QString ssh_key = CHubController::Instance().get_env_key(env.id());
  QString ip = cip.ip;
  QString port = cip.port;
  QString username = CSettingsManager::Instance().ssh_user();

  dlg_transfer_file->addIPPort(ip, port);
  dlg_transfer_file->addSSHKey(ssh_key);
  dlg_transfer_file->addUser(username);
  m_last_generated_tranferfile_dlg = dlg_transfer_file;
  show_dialog(last_generated_transferfile_dlg, "Transfer File " + env.name() + "/" + cont.name());
}

void TrayControlWindow::ssh_to_container_triggered(const CEnvironment &env,
                                                   const CHubContainer &cont) {
  qDebug()
      << QString("Environment [name: %1, id: %2]").arg(env.name(), env.id())
      << QString("Container [name: %1, id: %2]").arg(cont.name(), cont.id());
  CHubController::Instance().ssh_to_container_from_tray(env, cont);
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::ssh_to_rh_triggered(const QString &peer_fingerprint) {
  qDebug() << QString("Peer [peer_fingerprint: %1]").arg(peer_fingerprint);
  QtConcurrent::run(CRhController::Instance(), &CRhController::ssh_to_rh, peer_fingerprint);
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code) {
  qDebug()
      << QString("Peer [peer_fingerprint: %1]").arg(peer_fingerprint);

  if (res != SCWE_SUCCESS)
  {
    if (libbssh_exit_code != 0)
      CNotificationObserver::Info(tr("This Peer is not accessible with provided credentials. Please check and verify. Error SSH code: %1").arg(CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)libbssh_exit_code)),
                                  DlgNotification::N_NO_ACTION);
    else
      CNotificationObserver::Info(tr("Can't run terminal to ssh into peer. Error code: %1").arg(CSystemCallWrapper::scwe_error_to_str(res)),
                                  DlgNotification::N_NO_ACTION);
  }
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::desktop_to_container_triggered(const CEnvironment &env,
                                                   const CHubContainer &cont) {
  qDebug()
      << QString("Environment [name: %1, id: %2]").arg(env.name(), env.id())
      << QString("Container [name: %1, id: %2]").arg(cont.name(), cont.id())
      << QString("X2go is Launchable: %1").arg(CSystemCallWrapper::x2goclient_check());
  if (!CSystemCallWrapper::x2goclient_check()) {
    CNotificationObserver::Error(QObject::tr("X2Go-Client is not launchable. Make sure x2go-client is installed from \"About\" settings.")
                                 .arg(x2goclient_url()), DlgNotification::N_ABOUT);
    return;
  }
  CHubController::Instance().desktop_to_container_from_tray(env, cont);
}


////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::update_available(QString file_id) {
  qDebug() << "File ID: " << file_id;
  CNotificationObserver::Info(
      tr("Update for %1 is available. Check \"About\" dialog").arg(file_id), DlgNotification::N_ABOUT);
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::update_finished(QString file_id, bool success) {
  qDebug() << QString("File ID: %1, Success: %2").arg(file_id, success);
  if (!success) {
    CNotificationObserver::Error(
        tr("Failed to update %1. See details in error logs").arg(file_id), DlgNotification::N_NO_ACTION);
    return;
  }
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::launch_Hub() {
  CHubController::Instance().launch_browser(hub_site());
}
////////////////////////////////////////////////////////////////////////////

/*p2p status */
void TrayControlWindow::launch_p2p(){
    qDebug()
            <<"p2p button is pressed";
    switch (p2p_current_status) {
        case P2PStatus_checker::P2P_FAIL :
            CNotificationObserver::Error(QObject::tr("P2P is not installed. You can't connect to the environments without P2P."),
                                         DlgNotification::N_INSTALL_P2P);
            break;
        case P2PStatus_checker::P2P_READY :
            CNotificationObserver::Info(QObject::tr("P2P is not launched. "
                                                 "Press start to launch P2P daemon"), DlgNotification::N_START_P2P);
            break;
        case P2PStatus_checker::P2P_RUNNING :
            CNotificationObserver::Info(QObject::tr("P2P is running. Stop and start it, if you have troubles."), DlgNotification::N_STOP_P2P);
            break;
        case P2PStatus_checker::P2P_LOADING :
            CNotificationObserver::Info(QObject::tr("P2P daemon is loading"), DlgNotification::N_NO_ACTION);
            break;
        case P2PStatus_checker::P2P_INSTALLING:
            CNotificationObserver::Info(QObject::tr("P2P is installing"), DlgNotification::N_NO_ACTION);
            break;
    }
}

/*p2p installations */

void TrayControlWindow::launch_p2p_installation(){
    CHubController::Instance().launch_browser("https://subutai.io/install/index.html");
}

//////////////////////////////////////

void TrayControlWindow::balance_updated_sl() {
  m_act_balance->setText(CHubController::Instance().balance());
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::environments_updated_sl(int rr) {
  qDebug()
      << "Updating environments list"
      << "Result: " << rr;



  static QIcon unhealthy_icon(":/hub/BAD.png");
  static QIcon healthy_icon(":/hub/GOOD.png");
  static QIcon modification_icon(":/hub/OK.png");
  static QString deteted_string("DELETED");

  UNUSED_ARG(rr);
  static std::vector<QString> lst_checked_unhealthy_env;
  m_hub_menu->clear();

  std::map<QString, std::vector<QString> > tbl_envs;
  std::map<QString, int> new_envs;

  if (CHubController::Instance().lst_environments().empty()) {
    QAction* empty_action = new QAction("Empty", this);
    empty_action->setEnabled(false);
    m_hub_menu->addAction(empty_action);
    return;
  }

  for (auto env = CHubController::Instance().lst_environments().cbegin();
       env != CHubController::Instance().lst_environments().cend(); ++env) {
    QString env_id = env->id();
    environments_table[env_id] = *env;
    QString env_name = env->name();
    QAction* env_start = m_hub_menu->addAction(env->name());
    new_envs[env_name] = 1;
    env_start->setIcon(env->status() == "HEALTHY" ? healthy_icon :
                          env->status() == "UNHEALTHY" ? unhealthy_icon : modification_icon);

    std::vector<QString>::iterator iter_found =
        std::find(lst_checked_unhealthy_env.begin(),
                  lst_checked_unhealthy_env.end(), env->id());

    if (!env->healthy()) {
      if (iter_found == lst_checked_unhealthy_env.end()) {
        lst_checked_unhealthy_env.push_back(env->id());
        tbl_envs[env->status()].push_back(env_name);
        qCritical(
            "Environment %s, %s is unhealthy. Reason : %s",
            env_name.toStdString().c_str(), env->id().toStdString().c_str(),
            env->status_description().toStdString().c_str());
      }
    } else {
      if (iter_found != lst_checked_unhealthy_env.end()) {
        CNotificationObserver::Info(
            tr("Environment %1 became healthy").arg(env->name()), DlgNotification::N_NO_ACTION);
        qInfo(
            "Environment %s became healthy", env->name().toStdString().c_str());
        lst_checked_unhealthy_env.erase(iter_found);
      }
      qInfo(
          "Environment %s is healthy", env->name().toStdString().c_str());
    }
    connect(env_start, &QAction::triggered, [env, this](){
      this->generate_env_dlg(&(*env));
      TrayControlWindow::show_dialog(TrayControlWindow::last_generated_env_dlg,
                                     QString("Environment \"%1\" (%2)").arg(env->name()).arg(env->status()));
    });
  }

  for (std::map<QString, std::vector<QString> >::iterator it = tbl_envs.begin(); it != tbl_envs.end(); it++){
      if(!it->second.empty()){
          QString str_env_names = "";
          for (size_t i=0; i < it->second.size()-1; i++)
              str_env_names += it->second[i] + ", ";
          str_env_names += it->second[it->second.size()-1];
          QString str_notifications =
              tr("Environment%1 %2 %3 %4")
                  .arg(it->second.size() > 1 ? "s" : "")
                  .arg(str_env_names)
                  .arg(it->second.size() > 1 ? "are" : "is")
                  .arg(it->first);
          CNotificationObserver::Instance()->Info(str_notifications, DlgNotification::N_NO_ACTION);
      }
  }

  for (std::map<QString, CEnvironment>::iterator it = environments_table.begin();
       it != environments_table.end(); it++){
      if(new_envs[it->second.name()] == 0)
          it->second.set_status(deteted_string);
  }
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::my_peers_updated_sl() {
    in_peer_slot = true;
    QString msgDisconnected = "", msgOnline = "", msgOffline = "";
    //get peers list;
    std::vector<CMyPeerInfo> hub_peers = CHubController::Instance().lst_my_peers();
    std::vector<std::pair <QString, QString> > network_peers;
    for (std::pair<QString, QString> local_peer : CRhController::Instance()->dct_resource_hosts()){
        network_peers.push_back(std::make_pair(CCommons::GetFingerprintFromUid(local_peer.first), local_peer.second));
    }
    //update connected peers
    for (auto peer_info : hub_peers){
        if(hub_peers_table.find(peer_info.fingerprint()) == hub_peers_table.end()){
            peer_info.status() == "ONLINE" ? msgOnline += ", " + peer_info.name() : msgOffline += ", " + peer_info.name();
        }
        else if(hub_peers_table[peer_info.fingerprint()].status() != peer_info.status()){
            peer_info.status() == "ONLINE" ? msgOnline += ", " + peer_info.name() : msgOffline += ", " + peer_info.name();
        }
        peer_info.set_updated(true);
        hub_peers_table[peer_info.fingerprint()] = peer_info;
        update_peer_button(peer_info.fingerprint().toUpper(), peer_info);
    }
    for (auto peer_info : network_peers){
        network_peers_table[peer_info.first] = std::make_pair(peer_info.second, true);
        update_peer_button(peer_info.first.toUpper(), peer_info);
    }
    //delete disconnected peers
    std::vector<QString> hub_disconnected_peers;
    std::vector<QString> network_disconnected_peers;
    for (auto peer_it = hub_peers_table.begin(); peer_it != hub_peers_table.end(); peer_it++){
        if(!peer_it->second.updated()){
            hub_disconnected_peers.push_back(peer_it->second.fingerprint());
            msgDisconnected += ", " + peer_it->second.name();
        }
        else{
            peer_it->second.set_updated(false);
        }
    }
    for (auto peer_it = network_peers_table.end(); peer_it != network_peers_table.end(); peer_it++){
        if(!peer_it->second.second){
            network_disconnected_peers.push_back(peer_it->first);
        }
        else{
            peer_it->second.second = false;
        }
    }
    for (auto del_me : hub_disconnected_peers){
        hub_peers_table.erase(hub_peers_table.find(del_me));
        delete_peer_button_info(del_me.toUpper(), 1);
    }
    for (auto del_me : network_disconnected_peers){
        network_peers_table.erase(network_peers_table.find(del_me));
        delete_peer_button_info(del_me.toUpper(), 2);
    }
    //show notifications
    if(!msgOnline.isEmpty()){
        msgOnline.remove(0,2);
        CNotificationObserver::Instance()->Info(tr("%1 %2 online")
                                                .arg(msgOnline, msgOnline.contains(", ") ? "are" : "is"), DlgNotification::N_GO_TO_HUB);
    }
    if(!msgOffline.isEmpty()){
        msgOffline.remove(0,2);
        CNotificationObserver::Instance()->Info(tr("%1 %2 offline")
                                                .arg(msgOffline, msgOffline.contains(", ") ? "are" : "is"), DlgNotification::N_GO_TO_HUB);
    }
    if(!msgDisconnected.isEmpty()){
        msgDisconnected.remove(0,2);
        CNotificationObserver::Instance()->Info(tr("%1 %2 disconnected")
                                                .arg(msgDisconnected, msgDisconnected.contains(", ") ? "are" : "is"), DlgNotification::N_GO_TO_HUB);
    }
    in_peer_slot = false;
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::got_peer_info_sl(int type,
                                         QString name,
                                         QString dir,
                                         QString output){
    in_peer_slot = true;
    static QString new_updated = "new";

    if(type == 0 && name == "update" && dir == "peer" && output == "menu"){
        machine_peers_upd_finished();
        in_peer_slot = false;
        return;
    }
    if(CPeerController::Instance()->get_number_threads() <= 0){
        in_peer_slot = false;
        return;
    }
    CPeerController::Instance()->dec_number_threads();
    CLocalPeer updater_peer;
    if(machine_peers_table.find(name) != machine_peers_table.end())
        updater_peer = machine_peers_table[name];

    updater_peer.set_dir(dir);
    updater_peer.set_name(name);
    updater_peer.set_update(new_updated);

    switch (type) {
        case 0:
            updater_peer.set_status(output);
            break;
        case 1:
            updater_peer.set_ip(output);
            break;
        case 2:
            if(output != "undefined" && !output.isEmpty() && CSettingsManager::Instance().peer_finger(name) != output){
                delete_peer_button_info(CSettingsManager::Instance().peer_finger(name), 0);
                CSettingsManager::Instance().set_peer_finger(name, output);
            }
            updater_peer.set_fingerprint(output);
            break;
        default:
            break;
    }

    machine_peers_table[name] = updater_peer;
    if(!CSettingsManager::Instance().peer_finger(updater_peer.name()).isEmpty()){
        delete_peer_button_info(updater_peer.name(), 0);
        update_peer_button(CSettingsManager::Instance().peer_finger(updater_peer.name()), updater_peer);
    }
    else{
        update_peer_button(updater_peer.name(), updater_peer);
    }
    if(CPeerController::Instance()->get_number_threads() == 0){
        machine_peers_upd_finished();
    }
    in_peer_slot = false;
}

void TrayControlWindow::machine_peers_upd_finished(){
    qDebug()<<"refresh of local machine peers finished";
    std::vector <QString> delete_me;
    for(auto it = machine_peers_table.begin(); it != machine_peers_table.end(); it++){
        if(it->second.update() == "old"){
            delete_peer_button_info(CSettingsManager::Instance().peer_finger(it->second.name()), 0);
            delete_peer_button_info(it->second.name(), 0);
            delete_me.push_back(it->second.name());
        }
        else it->second.set_update("old");
    }
    for (auto del : delete_me){
        machine_peers_table.erase(machine_peers_table.find(del));
    }
}

void TrayControlWindow::peer_deleted_sl(const QString &peer_name){
    machine_peers_table.erase(machine_peers_table.find(peer_name));
    CPeerController::Instance()->finish_current_update();
}

void TrayControlWindow::peer_under_modification_sl(const QString &peer_name){
    static QString running_string = "running";
    static QString undefined_string = "undefined";
    if(machine_peers_table.find(peer_name) != machine_peers_table.end()){
        machine_peers_table[peer_name].set_status(running_string);
        machine_peers_table[peer_name].set_ip(undefined_string);
    }
    CPeerController::Instance()->finish_current_update();
}

void TrayControlWindow::peer_poweroff_sl(const QString &peer_name){
    static QString poweroff_string = "poweroff";
    static QString undefined_string = "undefined";
    if(machine_peers_table.find(peer_name) != machine_peers_table.end()){
        machine_peers_table[peer_name].set_status(poweroff_string);
        machine_peers_table[peer_name].set_ip(undefined_string);
    }
    CPeerController::Instance()->finish_current_update();
}

void TrayControlWindow::update_peer_button(const QString &peer_id, const CLocalPeer &peer_info){
    qDebug() << "update peer button information wih local peer";
    if(my_peers_button_table.find(peer_id) == my_peers_button_table.end()){
        my_peer_button *new_peer_button = new my_peer_button(peer_id, peer_info.name());
        my_peers_button_table[peer_id] = new_peer_button;
    }
    my_peer_button *peer_button = my_peers_button_table[peer_id];
    if(peer_button->m_local_peer == NULL){
        peer_button->m_local_peer = new CLocalPeer;
    }
    if(peer_info.status() == "running"){
        if(peer_info.ip() == "undefined" || peer_info.ip() == "loading" || peer_info.ip().isEmpty()){
            peer_button->m_local_peer_state = 3;
        }
        else if(peer_info.fingerprint() == "undefined" || peer_info.fingerprint() == "loading" || peer_info.fingerprint().isEmpty()){
            peer_button->m_local_peer_state = 3;
        }
        else{
            peer_button->m_local_peer_state = 1;
        }
    }
    else{
        peer_button->m_local_peer_state = 2;
    }
    if(peer_button->m_hub_peer == NULL){
        peer_button->peer_name = peer_info.name();
    }
    *(peer_button->m_local_peer) = peer_info;
    update_peer_icon(peer_id);
}

void TrayControlWindow::update_peer_button(const QString &peer_id, const CMyPeerInfo &peer_info){
    qDebug() << "update peer button information wih hub peer";
    if(my_peers_button_table.find(peer_id) == my_peers_button_table.end()){
        my_peer_button *new_peer_button = new my_peer_button(peer_id, peer_info.name());
        my_peers_button_table[peer_id] = new_peer_button;
    }
    my_peer_button *peer_button = my_peers_button_table[peer_id];
    if(peer_button->m_hub_peer == NULL){
        peer_button->m_hub_peer = new CMyPeerInfo;
    }
    if(peer_info.status() == "ONLINE" ){
       peer_button->m_hub_peer_state = 1;
    }
    else{
        peer_button->m_hub_peer_state = 2;
    }
    peer_button->peer_name = peer_info.name();
    *(peer_button->m_hub_peer) = peer_info;
    update_peer_icon(peer_id);
}

void TrayControlWindow::update_peer_button(const QString &peer_id, const std::pair<QString, QString> &peer_info){
    qDebug() << "update peer button information wih network peer(rh)";
    if(my_peers_button_table.find(peer_id) == my_peers_button_table.end()){
        my_peer_button *new_peer_button = new my_peer_button(peer_id, peer_info.second);
        my_peers_button_table[peer_id] = new_peer_button;
    }
    my_peer_button *peer_button = my_peers_button_table[peer_id];
    if((peer_button->m_hub_peer_state || peer_button->m_local_peer_state) == 0){
        peer_button->peer_name = peer_info.second;
    }
    if(peer_button->m_network_peer == NULL){
        peer_button->m_network_peer = new std::pair<QString, QString>;
    }
    *(peer_button->m_network_peer) = peer_info;
    peer_button->m_network_peer_state = 1;
    update_peer_icon(peer_id);
}

void TrayControlWindow::update_peer_icon(const QString &peer_id){
    qDebug() << "update peer icon: " <<peer_id;
    static QIcon online_icon(":/hub/GOOD.png");
    static QIcon offline_icon(":/hub/BAD.png");
    static QIcon unknown_icon(":/hub/OK.png");
    static QIcon local_hub(":/hub/local_hub.png");
    static QIcon local_network_icon(":/hub/local-network.png");
    static QIcon local_machine_off_icon(":/hub/local_off.png");
    static QIcon map_icons[2][3][4] = {
        { // no network peer(rh)
            {unknown_icon, local_network_icon, local_machine_off_icon, unknown_icon}, // no hub peer
            {online_icon, local_hub, local_machine_off_icon, unknown_icon}, // online hub peer
            {offline_icon, local_hub, local_machine_off_icon, unknown_icon}, // offline hub peer
        },
        { // found network peer(rh)
            {local_network_icon, local_network_icon, local_machine_off_icon, unknown_icon},
            {local_hub, local_hub, local_machine_off_icon, unknown_icon},
            {local_hub, local_network_icon, local_machine_off_icon}
        }
    };
    my_peer_button *peer_button = my_peers_button_table[peer_id];
    if(peer_button == NULL){
        return;
    }
    if(peer_button->m_my_peers_item == NULL){
        peer_button->m_my_peers_item = new QAction;
        connect(peer_button->m_my_peers_item, &QAction::triggered, [this, peer_button](){
            this->my_peer_button_pressed_sl(peer_button);
        });
    }
    peer_button->m_my_peers_item->setText(peer_button->peer_name);
    peer_button->m_my_peers_item->setIcon(map_icons[peer_button->m_network_peer_state][peer_button->m_hub_peer_state][peer_button->m_local_peer_state]);
    if( (peer_button->m_local_peer_state || peer_button->m_hub_peer_state ) == 0){ // no information about hub and local peers
        if(peer_button->m_my_peers_item != NULL){
            m_hub_peer_menu->removeAction(peer_button->m_my_peers_item);
            if(m_hub_peer_menu->actions().isEmpty()){
                m_hub_peer_menu->addAction(m_empty_action);
            }
        }
        if(peer_button->m_network_peer_state == 0){
            my_peers_button_table.erase(my_peers_button_table.find(peer_id));
        }else{
            peer_button->peer_name = peer_button->m_network_peer->second;
            peer_button->m_my_peers_item->setText(peer_button->peer_name);
            if(m_local_peer_menu->actions().indexOf(peer_button->m_my_peers_item) == -1){
                m_local_peer_menu->addAction(peer_button->m_my_peers_item);
                m_local_peer_menu->removeAction(m_empty_action);
            }
        }
        return;
    }else{
        m_local_peer_menu->removeAction(peer_button->m_my_peers_item);
        if(m_local_peer_menu->actions().empty()){
            m_local_peer_menu->addAction(m_empty_action);
        }
        if(m_hub_peer_menu->actions().indexOf(peer_button->m_my_peers_item) == -1){
            m_hub_peer_menu->addAction(peer_button->m_my_peers_item);
            m_hub_peer_menu->removeAction(m_empty_action);
        }
    }
}

void TrayControlWindow::delete_peer_button_info(const QString &peer_id, int type){
    if(my_peers_button_table.find(peer_id) == my_peers_button_table.end()){
        return;
    }
    my_peer_button *peer_button = my_peers_button_table[peer_id];
    switch (type) {
        case 0:
            if(peer_button->m_local_peer != NULL){
                delete peer_button->m_local_peer;
                peer_button->m_local_peer = NULL;
            }
            peer_button->m_local_peer_state = 0;
            break;
        case 1:
            if(peer_button->m_hub_peer != NULL){
                delete peer_button->m_hub_peer;
                peer_button->m_hub_peer = NULL;
            }
            peer_button->m_hub_peer_state = 0;
            break;
        case 2:
            if(peer_button->m_network_peer != NULL){
                delete peer_button->m_network_peer;
                peer_button->m_network_peer = NULL;
            }
            peer_button->m_network_peer_state = 0;
            break;
    }
    update_peer_icon(peer_id);
    if(peer_button->peer_name != peer_id && type == 0){
        CSettingsManager::Instance().set_peer_finger(peer_button->m_local_peer->name(), "");
    }
}

void TrayControlWindow::my_peer_button_pressed_sl(const my_peer_button *peer_info){
    if(peer_info == NULL){
        return;
    }
    static std::pair<QString, QString> empty_network_peer = std::make_pair("","");
    std::vector<CLocalPeer> local_peer_info;
    if(peer_info->m_local_peer != NULL){
        local_peer_info.push_back(*(peer_info->m_local_peer));
    }
    this->generate_peer_dlg(peer_info->m_hub_peer,
                            peer_info->m_network_peer == NULL ? empty_network_peer : *(peer_info->m_network_peer),
                            local_peer_info);
    TrayControlWindow::show_dialog(TrayControlWindow::last_generated_peer_dlg,
                                   QString("Peer \"%1\"").arg(peer_info->peer_name));
}
////////////////////////////////////////////////////////////////////////////

/* p2p status updater*/
void TrayControlWindow::update_p2p_status_sl(P2PStatus_checker::P2P_STATUS status){
    p2p_current_status=status;
    // need to put static icons
    static QIcon p2p_running(":/hub/running.png");
    static QIcon p2p_waiting(":/hub/waiting");
    static QIcon p2p_fail(":/hub/stopped");
    static QIcon p2p_loading(":/hub/loading");
    qDebug()
            <<"p2p updater got signal and try to update status";
    if(P2PStatus_checker::Instance().get_status() == P2PStatus_checker::P2P_INSTALLING){
        m_act_p2p_status->setText("P2P is installing");
        m_act_p2p_status->setIcon(p2p_loading);
        return;
    }
    switch(status){
        case P2PStatus_checker::P2P_READY :
            m_act_p2p_status->setText(tr("P2P is not running"));
            m_act_p2p_status->setIcon(p2p_waiting);
            break;
        case P2PStatus_checker::P2P_RUNNING :
            m_act_p2p_status->setText(tr("P2P is running"));
            m_act_p2p_status->setIcon(p2p_running);
            break;
        case P2PStatus_checker::P2P_FAIL :
            if(p2p_current_status == P2PStatus_checker::P2P_LOADING)
                CNotificationObserver::Error(QObject::tr("P2P is not installed. You can't connect to the environments without P2P."),
                                             DlgNotification::N_INSTALL_P2P);
            m_act_p2p_status->setText(tr("Can't launch P2P"));
            m_act_p2p_status->setIcon(p2p_fail);
            break;
        case P2PStatus_checker::P2P_LOADING :
            m_act_p2p_status->setText(tr("P2P is loading..."));
            m_act_p2p_status->setIcon(p2p_loading);
            break;
        case P2PStatus_checker::P2P_INSTALLING :
            m_act_p2p_status->setText(tr("P2P is installing"));
            m_act_p2p_status->setIcon(p2p_loading);
            break;
    }
}
//////////////////////////////////////

void TrayControlWindow::got_ss_console_readiness_sl(bool is_ready,
                                                    QString err) {
  qDebug()
      << "Is console ready: " << is_ready
      << "Error: " << err;
  if (!is_ready) {
    CNotificationObserver::Info(tr(err.toStdString().c_str()), DlgNotification::N_NO_ACTION);
    return;
  }

  QString hub_url = "https://localhost:9999";

  std::string rh_ip;
  int ec = 0;

  system_call_wrapper_error_t scwe = CSystemCallWrapper::get_rh_ip_via_libssh2(
      CSettingsManager::Instance().rh_host(m_default_peer_id).toStdString().c_str(),
      CSettingsManager::Instance().rh_port(m_default_peer_id),
      CSettingsManager::Instance().rh_user(m_default_peer_id).toStdString().c_str(),
      CSettingsManager::Instance().rh_pass(m_default_peer_id).toStdString().c_str(), ec, rh_ip);

  if (scwe == SCWE_SUCCESS && (ec == RLE_SUCCESS || ec == 0)) {
    hub_url = QString("https://%1:8443").arg(rh_ip.c_str());
  } else {
    qCritical(
        "Can't get RH IP address. Err : %s",
        CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)ec));
    CNotificationObserver::Info(
        tr("Can't get RH IP address. Error : %1")
            .arg(CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)ec)), DlgNotification::N_NO_ACTION);
    return;
  }

  CHubController::Instance().launch_browser(hub_url);
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::launch_ss() {
  std::string rh_ip;
  int ec = 0;
  system_call_wrapper_error_t scwe = CSystemCallWrapper::get_rh_ip_via_libssh2(
      CSettingsManager::Instance().rh_host(m_default_peer_id).toStdString().c_str(),
      CSettingsManager::Instance().rh_port(m_default_peer_id),
      CSettingsManager::Instance().rh_user(m_default_peer_id).toStdString().c_str(),
      CSettingsManager::Instance().rh_pass(m_default_peer_id).toStdString().c_str(), ec, rh_ip);

  if (scwe == SCWE_SUCCESS && (ec == RLE_SUCCESS || ec == 0)) {
    QString tmp =
        QString("https://%1:8443/rest/v1/peer/ready").arg(rh_ip.c_str());
    // after that got_ss_console_readiness_sl will be called
    qInfo("launch_ss : %s", tmp.toStdString().c_str());
    CRestWorker::Instance()->check_if_ss_console_is_ready(tmp);
  } else {
    qCritical(
        "Can't get RH IP address. Err : %s",
        CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)ec));
    CNotificationObserver::Info(tr("Can't get RH IP address. Error : %1")
            .arg(CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)ec)), DlgNotification::N_NO_ACTION);
  }
}
////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::show_dialog(QDialog* (*pf_dlg_create)(QWidget*),
                                    const QString& title) {
  std::map<QString, QDialog*>::iterator iter = m_dct_active_dialogs.find(title);
  qDebug() << "Poping up the dialog with title: " << title;
  if (iter == m_dct_active_dialogs.end()) {
    QDialog* dlg = pf_dlg_create(this);
    dlg->setWindowTitle(title);
    Qt::WindowFlags flags = 0;
    flags = Qt::Window;
    flags |= Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowMaximizeButtonHint;
    flags |= Qt::WindowCloseButtonHint;
    dlg->setWindowFlags(flags);
    m_dct_active_dialogs[dlg->windowTitle()] = dlg;

    int src_x, src_y, dst_x, dst_y;
    get_sys_tray_icon_coordinates_for_dialog(src_x, src_y, dst_x, dst_y,
                                             dlg->width(), dlg->height(), true);

    if (CSettingsManager::Instance().use_animations()) {
      QPropertyAnimation* pos_anim = new QPropertyAnimation(dlg, "pos");
      QPropertyAnimation* opa_anim =
          new QPropertyAnimation(dlg, "windowOpacity");

      pos_anim->setStartValue(QPoint(src_x, src_y));
      pos_anim->setEndValue(QPoint(dst_x, dst_y));
      pos_anim->setEasingCurve(QEasingCurve::OutBack);
      pos_anim->setDuration(800);

      opa_anim->setStartValue(0.0);
      opa_anim->setEndValue(1.0);
      opa_anim->setEasingCurve(QEasingCurve::Linear);
      opa_anim->setDuration(800);

      QParallelAnimationGroup* gr = new QParallelAnimationGroup;
      gr->addAnimation(pos_anim);
      gr->addAnimation(opa_anim);

      dlg->move(src_x, src_y);
      dlg->show();
      gr->start();
      connect(gr, &QParallelAnimationGroup::finished, [dlg]() {
        dlg->activateWindow();
        dlg->raise();
        dlg->setFocus();
      });
      connect(gr, &QParallelAnimationGroup::finished, gr,
              &QParallelAnimationGroup::deleteLater);
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

void TrayControlWindow::dialog_closed(int unused) {
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

QDialog* create_settings_dialog(QWidget* p) { return new DlgSettings(p); }
void TrayControlWindow::show_settings_dialog() {
  show_dialog(create_settings_dialog, tr("Settings"));
}
////////////////////////////////////////////////////////////////////////////

QDialog* create_create_peer_dialog(QWidget* p){ return new DlgCreatePeer(p); }
void TrayControlWindow::show_create_dialog() {
    QString vg_version, vb_version;
    CSystemCallWrapper::oracle_virtualbox_version(vb_version);
    CSystemCallWrapper::vagrant_version(vg_version);
    if(vg_version == "undefined"){
        CNotificationObserver::Instance()->Error(tr("Install Vagrant to create Peers"), DlgNotification::N_ABOUT);
        return;
    }
    if(vb_version == "undefined"){
        CNotificationObserver::Instance()->Error(tr("You don't have any hypervisor for Vagrant"), DlgNotification::N_ABOUT);
        return;
    }
    show_dialog(create_create_peer_dialog, tr("Create peer"));
}
////////////////////////////////////////////////////////////////////////////
QDialog* create_about_dialog(QWidget* p) { return new DlgAbout(p); }
void TrayControlWindow::show_about() {
  show_dialog(create_about_dialog, tr("About Subutai Control Center"));
}

////////////////////////////////////////////////////////////////////////////

QDialog* create_ssh_key_generate_dialog(QWidget* p) {
  return new DlgGenerateSshKey(p);
}
void TrayControlWindow::ssh_key_generate_triggered() {
  show_dialog(create_ssh_key_generate_dialog, tr("SSH Key Manager"));
}

QDialog* create_notifications_dialog(QWidget* p) {
  return new DlgNotifications(p);
}
void TrayControlWindow::show_notifications_triggered() {
  show_dialog(create_notifications_dialog, tr("Notifications history"));
}

////////////////////////////////////////////////////////////////////////////

QDialog* TrayControlWindow::m_last_generated_env_dlg = NULL;

QDialog* TrayControlWindow::last_generated_env_dlg(QWidget *p) {
  UNUSED_ARG(p);
  return m_last_generated_env_dlg;
}

void TrayControlWindow::generate_env_dlg(const CEnvironment *env){
  qDebug()
      << "Generating environment dialog... \n"
      << "Environment name: " << env->name();
  DlgEnvironment *dlg_env = new DlgEnvironment();
  dlg_env->addEnvironment(env);
  connect(dlg_env, &DlgEnvironment::upload_to_container_sig, this, &TrayControlWindow::upload_to_container_triggered);
  connect(dlg_env, &DlgEnvironment::ssh_to_container_sig, this, &TrayControlWindow::ssh_to_container_triggered);
  connect(dlg_env, &DlgEnvironment::desktop_to_container_sig, this, &TrayControlWindow::desktop_to_container_triggered);
  m_last_generated_env_dlg = dlg_env;
}
////////////////////////////////////////////////////////////////////////////

QDialog* TrayControlWindow::m_last_generated_tranferfile_dlg = NULL;

QDialog* TrayControlWindow::last_generated_transferfile_dlg(QWidget *p) {
  UNUSED_ARG(p);
  return m_last_generated_tranferfile_dlg;
}

#include "DlgTransferFile.h"

void TrayControlWindow::generate_transferfile_dlg(){
  qDebug()
      << "Generating new transferfile dialog";
  DlgTransferFile *dlg_transfer_file = new DlgTransferFile();
  m_last_generated_tranferfile_dlg = dlg_transfer_file;
}

////////////////////////////////////////////////////////////////////////////

QDialog* TrayControlWindow::m_last_generated_peer_dlg = NULL;

QDialog* TrayControlWindow::last_generated_peer_dlg(QWidget *p) {
  UNUSED_ARG(p);
  return m_last_generated_peer_dlg;
}

void TrayControlWindow::generate_peer_dlg(CMyPeerInfo *peer, std::pair<QString, QString> local_peer, std::vector<CLocalPeer> lp){ // local_peer -> pair of fingerprint and local ip
  DlgPeer *dlg_peer = new DlgPeer();
  dlg_peer->addPeer(peer , local_peer, lp);
  connect(dlg_peer, &DlgPeer::ssh_to_rh_sig, this, &TrayControlWindow::ssh_to_rh_triggered);
  connect(dlg_peer, &DlgPeer::peer_deleted, this, &TrayControlWindow::peer_deleted_sl);
  connect(dlg_peer, &DlgPeer::peer_modified, this, &TrayControlWindow::peer_under_modification_sl);
  m_last_generated_peer_dlg = dlg_peer;
}

////////////////////////////////////////////////////////////////////////////

void TrayControlWindow::ssh_to_container_finished(
    const CEnvironment &env,
    const CHubContainer &cont,
    int result) {
  UNUSED_ARG(env);
  UNUSED_ARG(cont);
  if (result != SDLE_SUCCESS) {
    CNotificationObserver::Error(
        tr("Can't ssh to container. Err : %1")
            .arg(CHubController::ssh_desktop_launch_err_to_str(result)), DlgNotification::N_NO_ACTION);
  }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void TrayControlWindow::desktop_to_container_finished(
    const CEnvironment &env,
    const CHubContainer &cont,
    int result) {
  UNUSED_ARG(env);
  UNUSED_ARG(cont);
  qDebug() << "Result " << result;
  if (result != SDLE_SUCCESS) {
    CNotificationObserver::Error(
        tr("Can't desktop to container. Err : %1")
            .arg(CHubController::ssh_desktop_launch_err_to_str(result)), DlgNotification::N_NO_ACTION);
  }
}
////////////////////////////////////////////////////////////////////////////
