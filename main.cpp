#include <QApplication>

/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"

#include "SystemCallWrapper.h"

void test_syscall_functions() {
//  CSystemCallWrapper::join_to_p2p_swarm("hash", "key", "10.10.10.1");
  CSystemCallWrapper::run_ssh_in_terminal("lezh1k", "192.168.2.100");
}

int main(int argc, char *argv[]) {
  test_syscall_functions();

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
  QApplication app(argc, argv);
  qRegisterMetaType<com::Bstr>("com::Bstr");

  DlgLogin dlg;
  dlg.setModal(true);
  dlg.exec();
  if (dlg.result() == QDialog::Rejected)
    return 0;

  CVBoxManagerSingleton::Instance()->init_com();
  TrayControlWindow tcw;
  return app.exec();
}
