#ifndef DLGNOTIFICATION_H
#define DLGNOTIFICATION_H

#include <QDialog>
#include <QTimer>


namespace Ui {
  class DlgNotification;
}


class QLabel;
class CNotificationObserver;



class DlgNotification : public QDialog
{
  Q_OBJECT

public:

  enum NOTIFICATION_ACTION_TYPE{N_UPDATE_P2P = 0, N_UPDATE_TRAY, N_UPDATE_RH, N_UPDATE_RHM,
                                N_SETTINGS, N_GO_TO_HUB, N_NOTF_HISTORY, N_RESTART_TRAY, N_ABOUT, N_NO_ACTION};


  explicit DlgNotification(size_t notification_level,
                           const QString& msg,
                           QWidget *parent = 0,
                           NOTIFICATION_ACTION_TYPE action_type = N_NO_ACTION);
  virtual ~DlgNotification();

  static int NOTIFICATIONS_COUNT;

private:
  Ui::DlgNotification *ui;
  QTimer m_close_timer;

private slots:
  void btn_close_released();
protected:
  QPoint lastPressPos;
  virtual void mousePressEvent(QMouseEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // DLGNOTIFICATION_H
