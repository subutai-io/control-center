#ifndef DLGNOTIFICATION_H
#define DLGNOTIFICATION_H

#include <QDialog>
#include <QTimer>
#include "NotificationObserver.h"

namespace Ui {
  class DlgNotification;
}

class QLabel;

class DlgNotification : public QDialog
{
  Q_OBJECT

public:
  explicit DlgNotification(CNotificationObserver::notification_level_t level,
                           const QString& msg,
                           QWidget *parent = 0);
  ~DlgNotification();

private:
  Ui::DlgNotification *ui;
  QTimer m_close_timer;

private slots:
  void btn_close_released();

};

#endif // DLGNOTIFICATION_H
