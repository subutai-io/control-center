#include<QApplication>
#include "Tester.h"

int
main(int argc, char *argv[]) {
    QApplication::setApplicationName("TestingTray");
    QApplication::setOrganizationName("subut.ai");
    QApplication app(argc, argv);
    Tester::Instance()->runAllTest();   
    return app.exec();;
}
