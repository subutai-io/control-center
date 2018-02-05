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
  void addEnvironment(const CEnvironment *env);
  ~DlgEnvironment();

private:
  Ui::DlgEnvironment *ui;
  CEnvironment env;
  std::map<QString,QPushButton*> ssh_buttons;
  std::map<QString,QPushButton*> desktop_buttons;
  std::map<QString,QPushButton*> upload_buttons;

  void addContainer(const CHubContainer*cont);
  void check_container_status(const CHubContainer *cont, bool &ssh_all, bool &desktop_all);
  void check_environment_status();
  void remote_acces(const CHubContainer &cont);
  void change_btn(QPushButton *btn, const QString tt_msg, bool enabled);

signals:
    void ssh_to_container_sig(const CEnvironment*, const CHubContainer*);
    void desktop_to_container_sig(const CEnvironment*, const CHubContainer*);
    void upload_to_container_sig(const CEnvironment*, const CHubContainer*);
};

#endif // DLGENVIRONMENT_H
