#ifndef DLGSETTINGS_H
#define DLGSETTINGS_H

#include <math.h>
#include <QDialog>
#include <QTabWidget>
#include <QEvent>
#include <QModelIndex>
#include <QTimer>

namespace Ui {
  class DlgSettings;
}

class QStandardItemModel;
class QStandardItem;
class QListView;

class TabResizeFilter : public QObject {
private:
  QTabWidget* m_target;

  static void expandingTypeStyleSheet(QTabWidget* tw) {
    int w = tw->width() - tw->count();  // don't know why. but looks OK only
    // with this -tw->count(). MAGIC!!!
    int wb = floor(w / (tw->count() * 1.0));
    int ws = w - wb * (tw->count() - 1);
    tw->setStyleSheet(QString("QTabBar::tab:!selected {width : %1px;}"
                              "QTabBar::tab:selected {width : %2px;}")
                      .arg(wb)
                      .arg(ws));
  }

public:
  TabResizeFilter(QTabWidget* target) : QObject(target), m_target(target) {}
  bool eventFilter(QObject*, QEvent* ev) {
    if (ev->type() == QEvent::Resize) expandingTypeStyleSheet(m_target);
    return false;
  }
};
////////////////////////////////////////////////////////////////////////////

class DlgSettings : public QDialog
{
  Q_OBJECT

public:
  explicit DlgSettings(QWidget *parent = 0);
  ~DlgSettings();

private:
  Ui::DlgSettings *ui;
  TabResizeFilter *m_tab_resize_filter;
  QStandardItemModel* m_model_resource_hosts;

  QTimer m_refresh_rh_list_timer;
  int m_refresh_rh_list_progress_val;
  void rebuild_rh_list_model();

private slots:
  void le_terminal_cmd_changed();
  void btn_ok_released();
  void btn_cancel_released();
  void btn_p2p_file_dialog_released();
  void btn_scp_command_released();
  void btn_ssh_command_released();
  void btn_virtualbox_command_release();
  void btn_x2goclient_command_released();
  void btn_ssh_keygen_command_released();
  void btn_logs_storage_released();
  void btn_ssh_keys_storage_released();
  void btn_refresh_rh_list_released();
  void btn_vagrant_command_released();

  void refresh_rh_list_timer_timeout();
  void lstv_resource_hosts_double_clicked(QModelIndex ix0);
  void resource_host_list_updated_sl(bool has_changes);
};

#endif // DLGSETTINGS_H
