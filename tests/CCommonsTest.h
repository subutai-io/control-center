#ifndef COMMONSTEST_H
#define COMMONSTEST_H

#include <QObject>

class CommonsTest : public QObject
{
Q_OBJECT

private slots:
    // CCommons::FileMd5
    void testFileMd5();
    void testFileMd5_data();

    // CCommons::NetworkErrorToString
    void testNetworkErrorToString();
    void testNetworkErrorToString_data();

    // CCommons::RestartTray
    /* UNABLE TO TEST */

    // CCommons::IsApplicationLaunchable
    void testIsApplicationLaunchable();
    void testIsApplicationLaunchable_data();

    // CCommons DefaultTerminals
    void testDefaultTerminals();

    // CCommons::HasRecommendedTerminalArg
    void testHasRecommendedTerminalArg();
    void testHasRecommendedTerminalArg_data();

};

#endif // COMMONSTEST_H
