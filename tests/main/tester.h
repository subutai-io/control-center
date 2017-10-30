#ifndef TESTER_H
#define TESTER_H

#include <QObject>
#include <QtTest/QTest>


class Tester
{
    typedef QList<QObject*> TestList;

public:
    static Tester *Instance() {
        static Tester tester;
        return &tester;
    }

    TestList& testList() {
       static TestList list;
       return list;
    }

    void addTest(QObject* object) {
        TestList&  list = testList();
        list.append(object);
    }

    int runAllTest(int argc, char *argv[]) {
        int ret = 0;
        foreach (QObject* test, testList())
            ret += QTest::qExec(test, argc, argv);
        return ret;
    }
};


#endif // TESTER_H
