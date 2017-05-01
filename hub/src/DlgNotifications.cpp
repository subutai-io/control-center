#include "DlgNotifications.h"
#include "ui_DlgNotifications.h"

#include <QAbstractItemView>
#include "NotificationLogger.h"
#include "Commons.h"

DlgNotifications::DlgNotifications(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgNotifications) {
  ui->setupUi(this);

  m_notifications_model = new DlgNotificationsTableModel(this);
  ui->tv_notifications->setAlternatingRowColors(true);
  ui->tv_notifications->setSortingEnabled(true);

  m_notification_sort_proxy_model = new DlgNotificationSortProxyModel;
  m_notification_sort_proxy_model->setSourceModel(m_notifications_model);

  ui->tv_notifications->setModel(m_notification_sort_proxy_model);
  ui->cb_full_info->setCheckState(Qt::Checked);

  connect(CNotificationLogger::Instance(), &CNotificationLogger::notifications_updated,
          this, &DlgNotifications::rebuild_model);
  connect(ui->cb_full_info, &QCheckBox::toggled, this, &DlgNotifications::chk_full_info_toggled);

  rebuild_model();
}

DlgNotifications::~DlgNotifications() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgNotifications::rebuild_model() {
  bool full_info = ui->cb_full_info->checkState() == Qt::Checked;
  if (full_info) {
    if (m_notifications_model->rowCount(QModelIndex()) > 0) {
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
      ui->tv_notifications->resizeRowsToContents();
    }
  } else {
    if (m_notifications_model->rowCount(QModelIndex()) > 0) {
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
      ui->tv_notifications->resizeRowsToContents();
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgNotifications::chk_full_info_toggled(bool checked) {
  UNUSED_ARG(checked);
  rebuild_model();
}
////////////////////////////////////////////////////////////////////////////
