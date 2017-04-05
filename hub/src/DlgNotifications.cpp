#include "DlgNotifications.h"
#include "ui_DlgNotifications.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractItemView>
#include "NotificationLogger.h"

DlgNotifications::DlgNotifications(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgNotifications) {
  ui->setupUi(this);

  m_model = new QStandardItemModel(this);
  ui->tv_notifications->setModel(m_model);

  ui->tv_notifications->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tv_notifications->setAlternatingRowColors(true);

  rebuild_model();
  connect(CNotificationLogger::Instance(), SIGNAL(notifications_updated()),
          this, SLOT(rebuild_model()));
}

DlgNotifications::~DlgNotifications() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgNotifications::rebuild_model() {
  int row_count = 0;
  for (auto i : CNotificationLogger::Instance()->notifications()) {
    QStandardItem *ni[3];
    ni[0] = new QStandardItem(i.date_time().toString(Qt::TextDate));
    ni[1] = new QStandardItem(i.level_str());
    ni[2] = new QStandardItem(i.message());
    for (int j = 0; j < 3; ++j) {
      static QColor color[4] = {QColor::fromRgb(145,255,200),
                                QColor::fromRgb(255,200,145),
                                QColor::fromRgb(255,145,145),
                                QColor::fromRgb(210,0,15)};
      ni[j]->setBackground(QBrush(color[(int)i.level()]));
      m_model->setItem(row_count, j, ni[j]);
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
  }
}
////////////////////////////////////////////////////////////////////////////
