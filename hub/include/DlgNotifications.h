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
};

#endif // DLGNOTIFICATIONS_H
