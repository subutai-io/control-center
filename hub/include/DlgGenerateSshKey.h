#ifndef DLGGENERATESSHKEY_H
#define DLGGENERATESSHKEY_H

#include <QDialog>

namespace Ui {
  class DlgGenerateSshKey;
}

class QStandardItemModel;
class QListView;

class DlgGenerateSshKeyInitializer : public QObject {
  Q_OBJECT
private:
public:
public slots:
  void start_initialization();
signals:

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

  std::map<QString, std::vector<bool> > m_dct_environment_keyflags;
  std::map<QString, std::vector<QString> > m_dct_key_environments;

  void generate_new_ssh();
  void refresh_key_files();

private slots:
  void btn_generate_released();
  void btn_send_to_hub_released();

  void btn_to_disabled_released();
  void btn_to_enabled_released();

  void cb_environments_index_changed(int ix);
  void environments_updated(int update_result);

};
#endif // DLGGENERATESSHKEY_H
