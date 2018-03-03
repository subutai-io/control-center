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

private:
  Ui::DlgCreatePeer *ui;
};

#endif // DLGCREATEPEER_H
