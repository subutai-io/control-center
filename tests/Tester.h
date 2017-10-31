#ifndef TESTER_H
#define TESTER_H

#include <QObject>

class Tester : public QObject
{
    Q_OBJECT
    typedef QList<QObject*> TestList;

public:
    Tester ();
    static Tester *Instance();
    int runAllTest();

private:
    TestList& testList();
    void addTest(QObject* object);

};


#endif // TESTER_H
