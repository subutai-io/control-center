#ifndef DLGENVIRONMENT_H
#define DLGENVIRONMENT_H

#include <QWidget>
#include "TrayControlWindow.h"


namespace Ui {
class DlgEnvironment;
}

class DlgEnvironment : public QDialog
{
    Q_OBJECT

public:
  explicit DlgEnvironment(QWidget *parent = 0);
  ~DlgEnvironment();
  void addContainer(const CHubContainer *cont);
  void addEnvironment(const CEnvironment *env);
  void set_button_ssh(QAction *act);
private:
    Ui::DlgEnvironment *ui;
private slots:
    void btn_ssh_all_clicked_sl();
signals:
    void btn_ssh_all_clicked();
};

#endif // DLGENVIRONMENT_H
