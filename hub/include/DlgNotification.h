#ifndef DLGNOTIFICATION_H
#define DLGNOTIFICATION_H

#include <QDialog>
#include <QTimer>
//#include "NotificationObserver.h"


namespace Ui {
  class DlgNotification;
}

class QLabel;
class CNotificationObserver;



class DlgNotification : public QDialog
{
  Q_OBJECT
public:

  enum NOTIFICATION_ACTION_TYPE{N_UPDATE_P2P = 0, N_UPDATE_TRAY, N_UPDATE_RH, N_UPDATE_RHM, N_NO_ACTION};
  explicit DlgNotification(size_t notification_level,
                           const QString& msg,
                           QWidget *parent = 0,
                           NOTIFICATION_ACTION_TYPE action_type = N_NO_ACTION);
  ~DlgNotification();
private:
  Ui::DlgNotification *ui;
  QTimer m_close_timer;

private slots:
  void btn_close_released();


};

#endif // DLGNOTIFICATION_H
