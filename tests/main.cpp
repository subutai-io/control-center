#include<QApplication>
#include "Tester.h"

int
main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Tester::Instance()->runAllTest();   
    return 0;
}
