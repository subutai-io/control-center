#ifndef DLGLOGIN_H
#define DLGLOGIN_H

#include <QDialog>
#include "RestWorker.h"

namespace Ui {
  class DlgLogin;
}

class DlgLogin : public QDialog
{
  Q_OBJECT

private:
  Ui::DlgLogin *ui;
  int m_login_count;

public:
  explicit DlgLogin(QWidget *parent = 0);
  ~DlgLogin();

private slots:
  void btn_ok_released();
  void btn_cancel_released();
};

#endif // DLGLOGIN_H
