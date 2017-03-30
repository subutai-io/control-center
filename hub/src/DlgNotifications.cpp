#include "DlgNotifications.h"
#include "ui_DlgNotifications.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include "NotificationLogger.h"

DlgNotifications::DlgNotifications(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgNotifications) {
  ui->setupUi(this);

  m_model = new QStandardItemModel(this);
  ui->tv_notifications->setModel(m_model);

  int row_count = 0;
  for (auto i : CNotificationLogger::Instance()->notifications()) {
    QStandardItem *ni_date, *ni_level, *ni_msg;
    ni_date = new QStandardItem(i.date_time().toString(Qt::ISODate));
    ni_level = new QStandardItem(i.level());
    ni_msg = new QStandardItem(i.message());

    m_model->setItem(row_count, 0, ni_date);
    m_model->setItem(row_count, 1, ni_level);
    m_model->setItem(row_count, 2, ni_msg);
    ++row_count;
  }

  m_model->setHeaderData(0, Qt::Horizontal, "Date");
  m_model->setHeaderData(1, Qt::Horizontal, "Level");
  m_model->setHeaderData(2, Qt::Horizontal, "Message");

  ui->tv_notifications->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  ui->tv_notifications->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
  ui->tv_notifications->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

DlgNotifications::~DlgNotifications() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////
