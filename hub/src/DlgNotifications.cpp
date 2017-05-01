#include "DlgNotifications.h"
#include "ui_DlgNotifications.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractItemView>
#include "NotificationLogger.h"
#include "Commons.h"

DlgNotifications::DlgNotifications(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgNotifications) {
  ui->setupUi(this);

  m_model = new QStandardItemModel(this);
  ui->tv_notifications->setModel(m_model);
  ui->tv_notifications->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tv_notifications->setAlternatingRowColors(true);

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
  static QColor color[4] = {QColor::fromRgb(145,255,200),
                            QColor::fromRgb(255,200,145),
                            QColor::fromRgb(255,145,145),
                            QColor::fromRgb(210,0,15)};
  int row_count = 0;
  m_model->clear();

  if (full_info) {
    for (auto i : CNotificationLogger::Instance()->notifications()) {
      QStandardItem *ni[3];
      QString ni_str[3] = {i.date_time().toString(Qt::TextDate),
                           i.level_str(), i.message()};

      for (int j = 0; j < 3; ++j) {
        ni[j] = new QStandardItem(ni_str[j]);
        ni[j]->setBackground(QBrush(color[(int)i.level()]));
        m_model->setItem(row_count, j, ni[j]);
        ni[j]->setEditable(false);
      }
      ++row_count;
    }

    m_model->setHeaderData(0, Qt::Horizontal, "Date");
    m_model->setHeaderData(1, Qt::Horizontal, "Level");
    m_model->setHeaderData(2, Qt::Horizontal, "Message");

    if (m_model->rowCount() > 0) {
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
      ui->tv_notifications->resizeRowsToContents();
    }
  } else {
    for (auto i : CNotificationLogger::Instance()->notification_unions()) {
      QStandardItem *ni[3]; //maybe more
      QString ni_str[3] = {i.level_str(), i.message(), QString("%1").arg(i.count())};
      for (int j = 0; j < 3; ++j) {
        ni[j] = new QStandardItem(ni_str[j]);
        ni[j]->setBackground(QBrush(color[(int)i.level()]));
        m_model->setItem(row_count, j, ni[j]);
        ni[j]->setEditable(false);
      }
      ++row_count;
    }

    m_model->setHeaderData(0, Qt::Horizontal, "Level");
    m_model->setHeaderData(1, Qt::Horizontal, "Message");
    m_model->setHeaderData(2, Qt::Horizontal, "Count");

    if (m_model->rowCount() > 0) {
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
