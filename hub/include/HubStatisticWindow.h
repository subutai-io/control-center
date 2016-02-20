#ifndef HUB_STATISTICS_WINDOW_H
#define HUB_STATISTICS_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>

#include "RestWorker.h"

namespace Ui {
  class HubStatisticWindow;
}

class QLabel;
class CTreeModelCSSContainer;
class CTreeModelSSPeerUser;

class HubStatisticWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit HubStatisticWindow(QWidget *parent = 0);
  ~HubStatisticWindow();
  void init_form();

private:
  Ui::HubStatisticWindow *ui;
  QTimer m_refresh_timer;
  CTreeModelCSSContainer* m_cont_model;
  CTreeModelSSPeerUser* m_peer_user_model;

  void adjust_lbl_font_size(QLabel *lbl);

private slots:
  void refresh_timer_timeout();
  void ma_settings_triggered(bool);
  void ma_quit_triggered(bool);

  // QWidget interface
protected:
  virtual void resizeEvent(QResizeEvent *event);
  virtual void showEvent(QShowEvent *event);
  virtual void closeEvent(QCloseEvent *event);
};

#endif // HUB_STATISTICS_WINDOW_H
