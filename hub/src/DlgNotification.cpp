#include <QCheckBox>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

#include "DlgNotification.h"
#include "SettingsManager.h"
#include "ui_DlgNotification.h"
#include "TrayControlWindow.h"
#include "updater/HubComponentsUpdater.h"
#include "Commons.h"


int DlgNotification::dlg_counter = 0;
QPoint DlgNotification::lastDialogPos = QPoint(0, 0);

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
    {"Update Subutai Tray",[](){update_system::CHubComponentsUpdater::Instance()->force_update_tray();}},
    {"Update Resource Host", [](){update_system::CHubComponentsUpdater::Instance()->force_update_rh();}},
    {"Update RH Management", [](){update_system::CHubComponentsUpdater::Instance()->force_update_rhm();}},
    {"Settings", [](){TrayControlWindow::Instance()->show_settings_dialog();}},
    {"SubutaiHub", [](){TrayControlWindow::Instance()->launch_Hub();}},
    {"Notification history", [](){TrayControlWindow::Instance()->show_notifications_triggered();}},
    {"Restart Tray", [](){CCommons::RestartTray();}},
    {"About", [](){TrayControlWindow::Instance()->show_about();}},
    {"No Action", [](){}}
  };

  if (action_type == N_NO_ACTION)
    ui->btn_action->setVisible(false);
  else {
   connect(ui->btn_action, &QPushButton::released,
            [action_type](){action_handler[action_type].call_func();});
   ui->btn_action->setText(action_handler[action_type].btn_message);
  }



  ui->lbl_icon->setAlignment(Qt::AlignCenter);
  ui->lbl_icon->setBackgroundRole(QPalette::Base);
  ui->lbl_icon->setAutoFillBackground(true);

  static QIcon icons[] = {QIcon(":/hub/info_notification.png"),
                          QIcon(":/hub/warning_notification.png"),
                          QIcon(":/hub/error_notification.png"),
                          QIcon(":/hub/Tray_icon_set-07.png")};
  static QString titles[] = {tr("Info"), tr("Warning"), tr("Error"),
                             tr("Critical")};
  this->setWindowTitle(titles[level]);
  this->setWindowIcon(icons[level]);
  QPixmap pixmap =
      icons[level].pixmap(ui->lbl_icon->width(), ui->lbl_icon->height());

  ui->lbl_icon->setPixmap(pixmap);
  ui->lbl_message->setText(msg);
  ui->lbl_message->setTextFormat(Qt::RichText);
  ui->lbl_message->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->lbl_message->setOpenExternalLinks(true);
  m_close_timer.setInterval(
      CSettingsManager::Instance().notification_delay_sec() * 500);
  ui->chk_ignore->setChecked(
      CSettingsManager::Instance().is_notification_ignored(msg));

  bool already_caled = false;
  connect(ui->btn_close, &QPushButton::released, [this, &already_caled](){
    if (!already_caled) {
      DlgNotification::dlg_counter --;
      already_caled = true;
      this->btn_close_released();
    }
  });

  connect(&m_close_timer, &QTimer::timeout, [this, &already_caled](){
      if (!already_caled) {
        DlgNotification::dlg_counter --;
        already_caled = true;
        this->close();
      }
  });

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

  DlgNotification::dlg_counter ++;
  qDebug() << dlg_counter;


  ui->chk_autohide->setChecked(true);
}

DlgNotification::~DlgNotification() { delete ui; }
////////////////////////////////////////////////////////////////////////////

void DlgNotification::btn_close_released() { close(); }
////////////////////////////////////////////////////////////////////////////
void DlgNotification::mousePressEvent(QMouseEvent *event){
    lastPressPos = this->pos() - event->globalPos();
}
////////////////////////////////////////////////////////////////////////////
void DlgNotification::mouseMoveEvent(QMouseEvent *event){
    this->move(event->globalPos() + lastPressPos);
}

