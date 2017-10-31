#ifndef TESTER_H
#define TESTER_H

#include <QObject>
#include <QtTest/QTest>
#include "commonstest.h"
#include <QDebug>

class Tester : public QObject
{
    Q_OBJECT
    typedef QList<QObject*> TestList;

public:
    Tester () {
      // adding all tests here
      addTest(new CommonsTest);
    }

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
        qDebug() << "adding test";

        list.append(object);
    }

    int runAllTest() {
        qDebug() << "run all tests\n";
        int ret = 0;
        foreach (QObject* test, testList())
            ret += QTest::qExec(test);
        return ret;
    }
};


#endif // TESTER_H
