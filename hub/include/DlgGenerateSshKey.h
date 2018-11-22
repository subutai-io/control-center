#ifndef DLGGENERATESSHKEY_H
#define DLGGENERATESSHKEY_H

#include <QDialog>
#include <QModelIndex>
#include <QMutex>
#include <map>

namespace Ui {
  class DlgGenerateSshKey;
}

class QStandardItemModel;
class QStandardItem;
class QListView;

struct EnvsSelectState {
  // environment id, state checked or unchecked
  std::map<QString, bool> states;
};

class DlgGenerateSshKey : public QDialog
{
  Q_OBJECT

public:
  explicit DlgGenerateSshKey(QWidget *parent = nullptr);
  ~DlgGenerateSshKey();

private:
  Ui::DlgGenerateSshKey *ui;
  QStandardItemModel* m_model_environments;
  QStandardItemModel* m_model_keys;
  bool m_change_everything_on_all_select;
  int m_current_key_index;
  // saves checked or unchecked environments
  // key: ssh key index, value: checked or unchecked
  std::map<int, EnvsSelectState> m_all_state_envs;

  void set_environments_checked_flag();
  void rebuild_environments_model();
  void rebuild_keys_model();

private slots:
  void btn_generate_released();
  void btn_remove_released();
  void btn_send_to_hub_released();

  void environments_item_changed(QStandardItem* item);

  void lstv_keys_current_changed(QModelIndex ix0, QModelIndex ix1);
  void ssh_key_send_progress_sl(int part, int total);
  void ssh_key_send_finished_sl();

  void chk_select_all_checked_changed(bool is_checked);

  void keys_updated_slot();
};
#endif // DLGGENERATESSHKEY_H
