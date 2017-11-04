#include "DownloadFileManagerTest.h"
#include "RestWorker.h"
#include "OsBranchConsts.h"
#include "DownloadFileManager.h"
#include <QTest>
#include <QTimer>
#include <QStandardPaths>

void DownloadFileManagerTest::testInstance() {
    QFETCH(QString, kurjun_file_id);
    QFETCH(QString, dst_file);
    QFETCH(int, expected_size);
    QFETCH(int, time_to_download);

    CDownloadFileManager *dfm = new CDownloadFileManager(kurjun_file_id, dst_file, expected_size);
    QTimer timer;
    QEventLoop loop;
    timer.setSingleShot(true);
    connect(dfm, &CDownloadFileManager::finished,
            &loop, &QEventLoop::quit);
    connect(dfm, &CDownloadFileManager::finished,
            &loop, &QEventLoop::quit);
    timer.start(time_to_download);
    dfm->start_download();
    loop.exec();
    QVERIFY(timer.isActive());

    QFileInfo fileInfo(dst_file);
    QVERIFY(fileInfo.exists());
    QVERIFY(fileInfo.size() == expected_size);
    loop.quit();
    timer.stop();
    dfm->deleteLater();
}

void DownloadFileManagerTest::testInstance_data() {
    QTest::addColumn<QString>("kurjun_file_id");
    QTest::addColumn<QString>("dst_file");
    QTest::addColumn<int>("expected_size");
    QTest::addColumn<int>("time_to_download");

    /// Test #1
    std::vector<CGorjunFileInfo> fi =
    CRestWorker::Instance()->get_gorjun_file_info(p2p_kurjun_file_name());
    QString dst_file =
         QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0] + QDir::separator() + p2p_kurjun_file_name();
    int time_to_download = 5000; // 5 seconds
    QTest::newRow("p2p kurjun file with location in temp directory")
        << fi.begin()->id()
        << dst_file
        << fi.begin()->size()
        << time_to_download;

    /// Test #2
    fi =
        CRestWorker::Instance()->get_gorjun_file_info(tray_kurjun_file_name());
    dst_file =
         QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0] + QDir::separator() + tray_kurjun_file_name();
    time_to_download = 2000; // 2 seconds
    QTest::newRow("tray kurjun file with location in temp directory")
        << fi.begin()->id()
        << dst_file
        << fi.begin()->size()
        << time_to_download;
}
