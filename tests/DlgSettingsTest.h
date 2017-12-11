#ifndef DLGSETTINGSTEST_H
#define DLGSETTINGSTEST_H

#include <QObject>

class DlgSettingsTest : public QObject
{
    Q_OBJECT

private slots:
    // DlgSettings
    void test_fill_log_level_combobox();

};

#endif // DLGSETTINGSTEST_H
