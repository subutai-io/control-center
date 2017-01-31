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

class DlgGenerateSshBackgroundWorker : public QObject {
  Q_OBJECT
private:
  std::map<QString, std::vector<QString> > m_dct_key_environments;
public:
  DlgGenerateSshBackgroundWorker(const std::map<QString,
                                 std::vector<QString> >& dct_key_environments);
  virtual ~DlgGenerateSshBackgroundWorker();
public slots:
  void start_initialization();
  void start_send_keys_to_hub();
signals:
  void send_key_progress(int part, int total);
  void send_key_finished();
};
////////////////////////////////////////////////////////////////////////////

class DlgGenerateSshKey : public QDialog
{
  Q_OBJECT

public:
  explicit DlgGenerateSshKey(QWidget *parent = 0);
  ~DlgGenerateSshKey();

private:
  Ui::DlgGenerateSshKey *ui;
  QStringList m_lst_key_files;
  QStringList m_lst_key_content;

  QStandardItemModel* m_model_environments;
  QStandardItemModel* m_model_keys;

  QString m_current_key;
  typedef std::map<QString, std::vector<bool> > map_string_bitmask;
  map_string_bitmask m_dct_environment_keyflags;
  map_string_bitmask m_dct_key_environments;
  map_string_bitmask m_dct_key_environments_original;

  void generate_new_ssh();
  void refresh_key_files();
  void rebuild_bitmasks();
  void set_environments_checked_flag();
  bool something_changed() const;
  void rebuild_environments_model();

private slots:
  void btn_generate_released();
  void btn_send_to_hub_released();

  void environments_updated(int update_result);
  void environments_item_changed(QStandardItem* item);

  void lstv_keys_current_changed(QModelIndex ix0, QModelIndex ix1);
  void ssh_key_send_progress(int part, int total);
};
#endif // DLGGENERATESSHKEY_H
