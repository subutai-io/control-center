#ifndef DOWNLOADFILEMANAGERTEST_H
#define DOWNLOADFILEMANAGERTEST_H

#include <QObject>

class DownloadFileManagerTest : public QObject
{
    Q_OBJECT
public:
    void createTestData();
private slots:

    void testInstance_data();
    void testInstance();

    void testInteruptFunction();
    void testInteruptFunction_data();
};

#endif // DOWNLOADFILEMANAGER_H
