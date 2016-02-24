#include "singleapplication.h"
#include <QApplication>

/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "SystemCallWrapper.h"

int main(int argc, char *argv[]) {

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
  //QApplication a(argc, argv);

  SingleApplication  a(argc, argv);


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
