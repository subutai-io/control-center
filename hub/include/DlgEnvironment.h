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
  void addContainer(const CHubContainer *cont);
  void addEnvironment(const CEnvironment *env);
  void addRemoteAccess(const CEnvironment *env, const CHubContainer *cont);
  void check_status(QPushButton *btn_ssh, QPushButton *btn_desktop, const CEnvironment *env, const CHubContainer *cont);
  void button_enhancement(QPushButton *btn);

  ~DlgEnvironment();

private:
    Ui::DlgEnvironment *ui;

signals:
    void ssh_to_container_sig(const CEnvironment*, const CHubContainer*, void*);
    void desktop_to_container_sig(const CEnvironment*, const CHubContainer*, void*);
};

#endif // DLGENVIRONMENT_H
