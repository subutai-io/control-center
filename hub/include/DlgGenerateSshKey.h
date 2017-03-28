#ifndef DLGGENERATESSHKEY_H
#define DLGGENERATESSHKEY_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
  class DlgGenerateSshKey;
}

class QStandardItemModel;
class QStandardItem;
class QListView;

class DlgGenerateSshKey : public QDialog
{
  Q_OBJECT

public:
  explicit DlgGenerateSshKey(QWidget *parent = 0);
  ~DlgGenerateSshKey();

private:
  Ui::DlgGenerateSshKey *ui;
  QStandardItemModel* m_model_environments;
  QStandardItemModel* m_model_keys;
  bool m_change_everything_on_all_select;

  void set_environments_checked_flag();
  void rebuild_environments_model();
  void rebuild_keys_model();

private slots:
  void btn_generate_released();
  void btn_send_to_hub_released();

  void environments_updated(int update_result);
  void environments_item_changed(QStandardItem* item);

  void lstv_keys_current_changed(QModelIndex ix0, QModelIndex ix1);
  void ssh_key_send_progress_sl(int part, int total);

  void chk_select_all_checked_changed(int st);    
};
#endif // DLGGENERATESSHKEY_H
