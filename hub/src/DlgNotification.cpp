#include <QCheckBox>
#include <QLabel>
#include <QMouseEvent>

#include "DlgNotification.h"
#include "SettingsManager.h"
#include "ui_DlgNotification.h"
#include "TrayControlWindow.h"
#include "updater/HubComponentsUpdater.h"
#include "Commons.h"


int DlgNotification::NOTIFICATIONS_COUNT = 0;

DlgNotification::DlgNotification(
   size_t notification_level, const QString &msg,
    QWidget *parent, NOTIFICATION_ACTION_TYPE action_type)
    : QDialog(parent, Qt::SplashScreen), ui(new Ui::DlgNotification) {

  CNotificationObserver::notification_level_t level = (CNotificationObserver::notification_level_t) notification_level;

  ui->setupUi(this);

  static struct handler_t {
    QString btn_message;
    std::function<void()> call_func;
  } action_handler[] = {
    {"Update P2P", [](){update_system::CHubComponentsUpdater::Instance()->force_update_p2p();}},
    {"Update",[](){update_system::CHubComponentsUpdater::Instance()->force_update_tray();}},
    {"Update Resource Host", [](){update_system::CHubComponentsUpdater::Instance()->force_update_rh();}},
    {"Update RH Management", [](){update_system::CHubComponentsUpdater::Instance()->force_update_rhm();}},
    {"Settings", [](){TrayControlWindow::Instance()->show_settings_dialog();}},
    {"Bazaar", [](){TrayControlWindow::Instance()->launch_Hub();}},
    {"Notification history", [](){TrayControlWindow::Instance()->show_notifications_triggered();}},
    {"Restart Control Center", [](){CCommons::RestartTray();}},
    {"About", [](){TrayControlWindow::Instance()->show_about();}},
    {"No Action", [](){}},
    {"Start P2P", [](){
      int rse_err;
      CSystemCallWrapper::restart_p2p_service(&rse_err, restart_p2p_type::STOPPED_P2P);
      if (rse_err == 0)
          CNotificationObserver::Instance()->Info(tr("Trying to launch P2P, wait 15 seconds"), DlgNotification::N_NO_ACTION);
      else
          CNotificationObserver::Error(QObject::tr("Can't launch p2p daemon. "
                                               "Either change the path setting in Settings or install the daemon if it is not installed. "
                                               "You can get the %1 daemon from <a href=\"%2\">here</a>.").
                                      arg(current_branch_name()).arg(p2p_package_url()), DlgNotification::N_SETTINGS);
      emit P2PStatus_checker::Instance().p2p_status(P2PStatus_checker::P2P_LOADING);}},
    {"Install P2P", [](){update_system::CHubComponentsUpdater::Instance()->install_p2p();}},
    {"Resolve", [](){system_call_wrapper_error_t res = CSystemCallWrapper::install_libssl(); UNUSED_ARG(res);}},
    {"Stop P2P", [](){
      int rse_err;
      CSystemCallWrapper::restart_p2p_service(&rse_err, restart_p2p_type::STARTED_P2P);
    }}
  };

  if (action_type == N_NO_ACTION)
    ui->btn_action->setVisible(false);
  else {
   connect(ui->btn_action, &QPushButton::released,
            [action_type, this](){action_handler[action_type].call_func();this->btn_close_released();});
   ui->btn_action->setText(action_handler[action_type].btn_message);

   // remove ignore checkbox on p2p action
   if (action_type == N_INSTALL_P2P ||
       action_type == N_START_P2P ||
       action_type == N_STOP_P2P ||
       action_type == N_UPDATE_P2P ||
       action_type == N_SETTINGS ||
       action_type == N_ABOUT) {
       ui->chk_ignore->hide();
   }
  }

  ui->lbl_icon->setAlignment(Qt::AlignCenter);
  ui->lbl_icon->setBackgroundRole(QPalette::Base);
  ui->lbl_icon->setAutoFillBackground(true);

  static QIcon icons[] = {QIcon(":/hub/info_notification.png"),
                          QIcon(":/hub/warning_notification.png"),
                          QIcon(":/hub/error_notification.png"),
                          QIcon(":/hub/Tray_icon_set-07.png")}; // need to change icon
  static QString titles[] = {tr("Info"), tr("Warning"), tr("Error"),
                             tr("Critical")};
  this->setWindowTitle(titles[level]);
  this->setWindowIcon(icons[level]);
  QPixmap pixmap =
      icons[level].pixmap(ui->lbl_icon->width(), ui->lbl_icon->height());

  ui->lbl_icon->setPixmap(pixmap);
  ui->lbl_message->acceptRichText();
  ui->lbl_message->setOpenExternalLinks(true);
  ui->lbl_message->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->lbl_message->setText(msg);
  ui->lbl_message->setStyleSheet("* { background-color: rgba(0, 0, 0, 0); border: none }");
  ui->lbl_message->setAlignment(Qt::AlignLeft);
  ui->lbl_message->setAlignment(Qt::AlignTop);
  m_close_timer.setInterval(
      CSettingsManager::Instance().notification_delay_sec() * 1000);
  ui->chk_ignore->setChecked(
      CSettingsManager::Instance().is_notification_ignored(msg));

  connect(ui->btn_close, &QPushButton::released, this, &DlgNotification::btn_close_released);
  connect(&m_close_timer, &QTimer::timeout, this, &DlgNotification::close);

  connect(ui->chk_autohide, &QCheckBox::stateChanged, [this](int state) {
    if (state == Qt::Checked)
      m_close_timer.start();
    else
      m_close_timer.stop();
  });

  connect(ui->chk_ignore, &QCheckBox::stateChanged, [msg](int state) {
    if (state == Qt::Checked)
      CSettingsManager::Instance().ignore_notification(msg);
    else
      CSettingsManager::Instance().not_ignore_notification(msg);
  });

  ++NOTIFICATIONS_COUNT;
  ui->chk_autohide->setChecked(true);
}

DlgNotification::~DlgNotification() {
  --NOTIFICATIONS_COUNT;
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgNotification::btn_close_released() {
  close();
}
////////////////////////////////////////////////////////////////////////////
void DlgNotification::mousePressEvent(QMouseEvent *event){
    m_lastPressPos = this->pos() - event->globalPos();
}
////////////////////////////////////////////////////////////////////////////
void DlgNotification::mouseMoveEvent(QMouseEvent *event){
    this->move(event->globalPos() + m_lastPressPos);
}

