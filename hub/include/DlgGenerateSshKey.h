#ifndef DLGGENERATESSHKEY_H
#define DLGGENERATESSHKEY_H

#include <QDialog>

namespace Ui {
  class DlgGenerateSshKey;
}

class QGridLayout;
class QHBoxLayout;
class QPushButton;
class QComboBox;
class QLabel;
class CEnvironmentEx;
class SshKeyWidget;

/*!
 * \brief This class is used for managing behavior of "Generate SSH key" dialog
 */
class DlgGenerateSshKey : public QDialog
{
  Q_OBJECT

public:
  explicit DlgGenerateSshKey(QWidget *parent = 0);
  ~DlgGenerateSshKey();

private:
  Ui::DlgGenerateSshKey *ui;

  QGridLayout* m_layout_grid;
  QHBoxLayout* m_layout_hbox;
  QPushButton* m_btn_generate;

  void generate_new_ssh();
  void set_key_text();

  enum ssh_key_paths {
    SP_PUB_KEY = 0, SP_PRIV_KEY, SP_STANDARD_PUB_KEY, SP_STANDARD_PRIV_KEY
  };

  static QString ssh_key_path(ssh_key_paths path);

private slots:
  void btn_generate_released();

};
#endif // DLGGENERATESSHKEY_H
