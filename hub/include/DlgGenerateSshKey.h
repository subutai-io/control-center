#ifndef DLGGENERATESSHKEY_H
#define DLGGENERATESSHKEY_H

#include <QDialog>

namespace Ui {
  class DlgGenerateSshKey;
}

class DlgGenerateSshKey : public QDialog
{
  Q_OBJECT

public:
  explicit DlgGenerateSshKey(QWidget *parent = 0);
  ~DlgGenerateSshKey();

private:
  Ui::DlgGenerateSshKey *ui;
  void generate_new_ssh();
  void set_key_text();

private slots:
  void btn_generate_released();
  void btn_copy_to_clipboard_released();
};

#endif // DLGGENERATESSHKEY_H
