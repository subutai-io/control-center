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

class TabResizeFilter;
/*!
 * \brief This class is used for managing "Settings" dialog
 */
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
  void btn_ok_released();
  void btn_cancel_released();
  void btn_p2p_file_dialog_released();
  void btn_ssh_command_released();
  void btn_vboxmanage_command_released();
  void btn_logs_storage_released();
  void btn_ssh_keys_storage_released();
  void btn_rtm_db_folder_released();
  void btn_refresh_rh_list_released();

  void refresh_rh_list_timer_timeout();
  void lstv_resource_hosts_double_clicked(QModelIndex ix0);
  void resource_host_list_updated_sl(bool has_changes);
};

#endif // DLGSETTINGS_H
