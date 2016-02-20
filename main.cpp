#include <QApplication>

/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  qRegisterMetaType<com::Bstr>("com::Bstr");

  DlgLogin dlg;
  dlg.setModal(true);
  dlg.exec();
  if (dlg.result() == QDialog::Rejected)
    return 0;

  CVBoxManagerSingleton::Instance()->init_com();
  TrayControlWindow tcw;
  int result = a.exec();
  return result;
}
