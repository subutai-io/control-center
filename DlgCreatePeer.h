#ifndef DLGCREATEPEER_H
#define DLGCREATEPEER_H

#include <QDialog>

namespace Ui {
  class DlgCreatePeer;
}

class DlgCreatePeer : public QDialog
{
  Q_OBJECT

public:
  explicit DlgCreatePeer(QWidget *parent = 0);
  ~DlgCreatePeer();
  QString create_dir(const QString &name);

private:
  Ui::DlgCreatePeer *ui;

public slots:
  void create_button_pressed();
};

#endif // DLGCREATEPEER_H
