#include <QLabel>

#include "DlgNotification.h"
#include "ui_DlgNotification.h"
#include "SettingsManager.h"

DlgNotification::DlgNotification(CNotificationObserver::notification_level_t level,
                                 const QString &msg,
                                 QWidget *parent) :
  QDialog(parent, Qt::SplashScreen),
  ui(new Ui::DlgNotification) {
  ui->setupUi(this);

  ui->lbl_icon->setAlignment(Qt::AlignCenter);
  ui->lbl_icon->setBackgroundRole(QPalette::Base);
  ui->lbl_icon->setAutoFillBackground(true);

  static QIcon icons[] = { QIcon(":/hub/info_notification.png"),
                           QIcon(":/hub/warning_notification.png"),
                           QIcon(":/hub/error_notification.png"),
                           QIcon(":/hub/Tray_icon_set-07.png") };
  static QString titles[] = {tr("Info"),
                             tr("Warning"),
                             tr("Error"),
                             tr("Critical")};
  this->setWindowTitle(titles[level]);
  this->setWindowIcon(icons[level]);
  QPixmap pixmap = icons[level].pixmap(ui->lbl_icon->width(),
                                       ui->lbl_icon->height());

  ui->lbl_icon->setPixmap(pixmap);
  ui->lbl_message->setText(msg);
  ui->lbl_message->setTextFormat(Qt::RichText);
  ui->lbl_message->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->lbl_message->setOpenExternalLinks(true);

  m_close_timer.setInterval(CSettingsManager::Instance().notification_delay_sec() * 1000);
  ui->chk_ignore->setChecked(CSettingsManager::Instance().is_notification_ignored(msg) ? Qt::Checked : Qt::Unchecked);

  connect(ui->btn_close, &QPushButton::released, this, &DlgNotification::btn_close_released);
  connect(&m_close_timer, &QTimer::timeout, this, &DlgNotification::close);
  connect(ui->chk_autohide, &QCheckBox::stateChanged, [this](int state) {
    if (state == Qt::Checked) m_close_timer.start();
    else m_close_timer.stop();
  });

  connect(ui->chk_ignore, &QCheckBox::stateChanged, [msg](int state) {
    if (state == Qt::Checked) CSettingsManager::Instance().ignore_notification(msg);
    else CSettingsManager::Instance().not_ignore_notification(msg);
  });

  ui->chk_autohide->setChecked(true);
}

DlgNotification::~DlgNotification() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgNotification::btn_close_released() {
  close();
}
////////////////////////////////////////////////////////////////////////////

