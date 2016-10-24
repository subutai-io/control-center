#ifndef DLGLOGIN_H
#define DLGLOGIN_H

#include <QDialog>

namespace Ui {
  class DlgLogin;
}

class QSplashScreen;

/*!
 * \brief This class is used for managing behavior of authorization dialog.
 */
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

  /*!
   * \brief Run dialog in different modes. When "Remember me" flag checked - it tries to login without dialog.
   */
  void run_dialog(QSplashScreen *sc);

private slots:
  void btn_ok_released();
  void btn_cancel_released();
  void cb_show_pass_state_changed(int st);

signals:
  void login_success();
};

#endif // DLGLOGIN_H
