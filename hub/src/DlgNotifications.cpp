#include <QPainter>
#include <QTextDocument>

#include "DlgNotifications.h"
#include "ui_DlgNotifications.h"
#include "NotificationLogger.h"
#include "Commons.h"

DlgNotifications::DlgNotifications(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgNotifications) {
  ui->setupUi(this);

  this->ui->tv_notifications->setItemDelegate(new Delegate());

  m_notifications_model = new DlgNotificationsTableModel(this);
  m_notification_unions_model = new DlgNotificationUnionsTableModel(this);

  m_notification_sort_proxy_model = new DlgNotificationSortProxyModel;
  m_notification_sort_proxy_model->setSourceModel(m_notifications_model);

  m_notification_unions_sort_proxy_model = new DlgNotificationUnionSortProxyModel;
  m_notification_unions_sort_proxy_model->setSourceModel(m_notification_unions_model);

  ui->tv_notifications->setAlternatingRowColors(true);
  ui->tv_notifications->setSortingEnabled(true);

  ui->tv_notifications->setModel(m_notification_sort_proxy_model);
  ui->cb_full_info->setCheckState(Qt::Checked);

  connect(CNotificationLogger::Instance(), &CNotificationLogger::notifications_updated,
          this, &DlgNotifications::rebuild_model);
  connect(ui->cb_full_info, &QCheckBox::toggled, this, &DlgNotifications::chk_full_info_toggled);

  rebuild_model();
}

DlgNotifications::~DlgNotifications() {
  delete ui;
  delete m_notifications_model;
  delete m_notification_unions_model;
  delete m_notification_sort_proxy_model;
  delete m_notification_unions_sort_proxy_model;
}
////////////////////////////////////////////////////////////////////////////

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const {
  QStyleOptionViewItem options = option;
  initStyleOption(&options, index);

  painter->save();

  QTextDocument doc;
  doc.setHtml(options.text);

  options.text = "";
  options.widget->style()->drawControl(QStyle::CE_ItemViewItem,
                                       &options, painter);

  painter->translate(options.rect.left(), options.rect.top());
  QRect clip(0, 0, options.rect.width(), options.rect.height());
  doc.drawContents(painter, clip);

  painter->restore();
}

////////////////////////////////////////////////////////////////////////////

void
DlgNotifications::rebuild_model() {
  bool full_info = ui->cb_full_info->checkState() == Qt::Checked;  
  if (full_info) {    
    ui->tv_notifications->setModel(m_notification_sort_proxy_model);
    if (m_notifications_model->rowCount(QModelIndex()) > 0) {
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
      ui->tv_notifications->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
      ui->tv_notifications->setSelectionMode(QAbstractItemView::NoSelection);
      ui->tv_notifications->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      ui->tv_notifications->sortByColumn(0);
    }
    m_notification_sort_proxy_model->invalidate();
  } else {
    ui->tv_notifications->setModel(m_notification_unions_sort_proxy_model);
    if (m_notification_unions_model->rowCount(QModelIndex()) > 0) {
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
      ui->tv_notifications->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
      ui->tv_notifications->resizeRowsToContents();
    }
    m_notification_unions_sort_proxy_model->invalidate();
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgNotifications::chk_full_info_toggled(bool checked) {
  UNUSED_ARG(checked);
  rebuild_model();
}
////////////////////////////////////////////////////////////////////////////
