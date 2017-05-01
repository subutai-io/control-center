#ifndef DLGNOTIFICATIONS_H
#define DLGNOTIFICATIONS_H

#include <QDialog>

class QStandardItemModel;
class QStandardItem;

namespace Ui {
  class DlgNotifications;
}

class DlgNotifications : public QDialog
{
  Q_OBJECT

public:
  explicit DlgNotifications(QWidget *parent = 0);
  ~DlgNotifications();

private:
  Ui::DlgNotifications *ui;
  QStandardItemModel *m_model;

private slots:
  void rebuild_model();
  void chk_full_info_toggled(bool checked);
};

#endif // DLGNOTIFICATIONS_H
