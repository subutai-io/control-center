#include "singleapplication.h"
#include <QApplication>

/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "SystemCallWrapper.h"
#include <QObject>

int main(int argc, char *argv[]) {

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
//  SingleApplication app(argc, argv);
  QApplication app(argc, argv);

  qRegisterMetaType<com::Bstr>("com::Bstr");

  DlgLogin dlg;
  dlg.setModal(true);
  dlg.exec();
  if (dlg.result() == QDialog::Rejected)
    return 0;

  CVBoxManagerSingleton::Instance()->init_com();
  TrayControlWindow tcw;

//  QObject::connect(QApplication::instance(), SIGNAL(showUp()), &tcw, SLOT(raise())); // window is your QWindow instance
//  QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
//                      &app, SLOT(handleMessage(const QString&)));

  return app.exec();
}
