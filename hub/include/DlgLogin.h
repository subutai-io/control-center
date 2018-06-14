#ifndef DLGLOGIN_H
#define DLGLOGIN_H

#include <QDialog>

namespace Ui {
  class DlgLogin;
}

class QSplashScreen;

class DlgLogin : public QDialog
{
  Q_OBJECT

private:
  Ui::DlgLogin *ui;
  QAction *m_show_password_action;
  int m_login_count;

  bool login();

public:
  explicit DlgLogin(QWidget *parent = 0);
  ~DlgLogin();

  /*!
   * \brief Run dialog in different modes. When "Remember me" flag checked - it tries to login without dialog.
   */
  void run_dialog(QSplashScreen *sc);
  void solve_libssl();
  bool m_password_state;

private slots:
  void btn_ok_released();
  void btn_cancel_released();

signals:
  void login_success();
};

#endif // DLGLOGIN_H
