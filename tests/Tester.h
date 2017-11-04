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
    void runAllTest();
    void runLast();

private:
    TestList& testList();
    void addTest(QObject* object);
};


#endif // TESTER_H
