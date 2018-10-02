#ifndef DLGCREATEFOLDER_H
#define DLGCREATEFOLDER_H

#include <QDialog>

namespace Ui {
  class DlgCreateFolder;
}

class DlgCreateFolder : public QDialog {
  Q_OBJECT

private:
  Ui::DlgCreateFolder *ui;
  QStringList existing_files;

  void btn_create_released();
  void btn_cancel_released();

public:
  explicit DlgCreateFolder(QWidget *parent = nullptr);
  ~DlgCreateFolder();

  void add_existing_files(const QString &file);
  void set_directory(const QString &type, const QString &dir);

signals:
  void got_new_folder_name(bool proceed, QString name);
};

#endif // DLGCREATEFOLDER_H
