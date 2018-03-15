#ifndef DLGENVIRONMENT_H
#define DLGENVIRONMENT_H

#include <QWidget>
#include <QCheckBox>
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
  std::map<QString,QCheckBox*> selected_conts;
  std::map<QString,QLabel*> desktops_info;

  void addContainer(const CHubContainer*cont);
  void check_container_status(const CHubContainer *cont);
  void check_environment_status();
  void remote_acces(const CHubContainer &cont);
  void change_cont_status(const CHubContainer *cont, int status);
  void upload_selected();
  void desktop_selected();
  void ssh_selected();
  void select_all(bool checked);
  void check_buttons();

  // staff that i need to delete
  std::vector <QTimer*> timers;
  std::vector <QLabel*> labels;
  std::vector <QCheckBox*> checkboxs;

signals:
    void ssh_to_container_sig(const CEnvironment*, const CHubContainer*);
    void desktop_to_container_sig(const CEnvironment*, const CHubContainer*);
    void upload_to_container_sig(const CEnvironment*, const CHubContainer*);
};

#endif // DLGENVIRONMENT_H
