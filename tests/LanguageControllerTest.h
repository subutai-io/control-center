#ifndef LANGUAGECONTROLLERTEST_H
#define LANGUAGECONTROLLERTEST_H

#include <QObject>

class LanguageControllerTest : public QObject
{
    Q_OBJECT
private slots:
    void testLocaleTypeToVal();
    void testLocaleTypeToVal_data();

};

#endif // LANGUAGECONTROLLERTEST_H
