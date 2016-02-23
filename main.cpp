#include <QApplication>

/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "SystemCallWrapper.h"

//#include <iostream>
//void test_join_2_swarm() {
//  std::vector<std::string> lst_output = CSystemCallWrapper::join_to_p2p_swarm("hash",
//                                                                              "key",
//                                                                              "10.10.10.1",
//                                                                              "/home/lezh1k/SRC/GO/src/p2p/p2p");
//  for (auto line = lst_output.begin(); line != lst_output.end(); ++line)
//    std::cout << *line << std::endl;
//}
////////////////////////////////////////////////////////////////////////////

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
