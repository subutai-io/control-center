#ifndef DLGNOTIFICATIONS_H
#define DLGNOTIFICATIONS_H

#include <QDialog>
#include "DlgNotificationsModel.h"
namespace Ui {
  class DlgNotifications;
}

class DlgNotifications : public QDialog
{
  Q_OBJECT

public:
  explicit DlgNotifications(QWidget *parent = 0);
  ~DlgNotifications();

private:
  Ui::DlgNotifications *ui;
  DlgNotificationsTableModel *m_notifications_model;
  DlgNotificationSortProxyModel *m_notification_sort_proxy_model;

  DlgNotificationUnionsTableModel *m_notification_unions_model;
  DlgNotificationUnionSortProxyModel *m_notification_unions_sort_proxy_model;

private slots:
  void rebuild_model();
  void chk_full_info_toggled(bool checked);
};

#endif // DLGNOTIFICATIONS_H
