#ifndef DLGLOGIN_H
#define DLGLOGIN_H

#include <QDialog>

namespace Ui {
  class DlgLogin;
}

class DlgLogin : public QDialog
{
  Q_OBJECT

private:
  Ui::DlgLogin *ui;
  int m_login_count;

  bool login();

public:
  explicit DlgLogin(QWidget *parent = 0);
  ~DlgLogin();
  void run_dialog();

private slots:
  void btn_ok_released();
  void btn_cancel_released();
  void cb_show_pass_state_changed(int st);
};

#endif // DLGLOGIN_H
