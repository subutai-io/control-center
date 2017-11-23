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
  void addRemoteAccess(const CEnvironment *env, const CHubContainer *cont);

private:
    Ui::DlgEnvironment *ui;
signals:
    void ssh_to_container_sig(const CEnvironment*, const CHubContainer*, void*);
    void desktop_to_container_sig(const CEnvironment*, const CHubContainer*);
};

#endif // DLGENVIRONMENT_H
