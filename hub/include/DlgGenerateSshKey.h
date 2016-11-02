#ifndef DLGGENERATESSHKEY_H
#define DLGGENERATESSHKEY_H

#include <QDialog>

namespace Ui {
  class DlgGenerateSshKey;
}

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
  bool m_standard_key_used;
  void generate_new_ssh();
  void set_key_text();

  QString ssh_pub_key_path() const;
  QString ssh_private_key_path() const;
  QString ssh_standard_pub_key_path() const;
  QString ssh_standard_private_key_path() const;

private slots:
  void btn_generate_released();
  void btn_copy_to_clipboard_released();
  void btn_add_to_environments_released();
};

#endif // DLGGENERATESSHKEY_H
