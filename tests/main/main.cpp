#include "tester.h"

int main(int argc, char *argv[])
{
    Tester::Instance()->addTest(nullptr);

    Tester::Instance()->runAllTest(argc, argv);
    return 0;

}
