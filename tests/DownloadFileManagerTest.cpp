#include "DownloadFileManagerTest.h"
#include "RestWorker.h"
#include "OsBranchConsts.h"
#include "DownloadFileManager.h"
#include <QTest>
#include <QTimer>
#include <QStandardPaths>
#include <QSignalSpy>

void DownloadFileManagerTest::testInstance() {
    QFETCH(QString, kurjun_file_id);
    QFETCH(QString, dst_file);
    QFETCH(int, expected_size);
    QFETCH(int, time_to_download);

    CDownloadFileManager *dfm = new CDownloadFileManager(kurjun_file_id, dst_file, expected_size);
    QTimer timer;
    QEventLoop loop;
    QSignalSpy spyFinished(dfm, &CDownloadFileManager::finished);
    QSignalSpy spyProgress(dfm, &CDownloadFileManager::download_progress_sig);

    timer.setSingleShot(true);
    connect(dfm, &CDownloadFileManager::finished,
            &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout,
            &loop, &QEventLoop::quit);
    timer.start(time_to_download);
    dfm->start_download();
    loop.exec();

    QVERIFY(timer.isActive()); // timeout for provided time to download. Something wrong with internet or it is too slow

    QCOMPARE(spyFinished.count() , 1); // check if signal 'finished' was emmited
    QVERIFY(spyProgress.count() > 0); // check if download_progress_signal was ever emitted

    QFileInfo fileInfo(dst_file);
    QVERIFY(fileInfo.exists()); // check if file was saved
    QVERIFY(fileInfo.size() == expected_size); // check if file was not corrupted
    loop.quit();
    timer.stop();
    dfm->deleteLater();
}


void DownloadFileManagerTest::testInteruptFunction() {
    QFETCH(QString, kurjun_file_id);
    QFETCH(QString, dst_file);
    QFETCH(int, expected_size);

    QFile old_file(dst_file);
    if (old_file.exists() && !old_file.remove()) // if we can't delete, we can't test this case
        return;

    CDownloadFileManager *dfm = new CDownloadFileManager(kurjun_file_id, dst_file, expected_size);
    QEventLoop loop;
    connect(dfm, &CDownloadFileManager::finished,
            &loop, &QEventLoop::quit);
    dfm->start_download();
    dfm->interrupt_download();

    QFileInfo fileInfo(dst_file);
    if (fileInfo.exists()){
        QVERIFY(fileInfo.size() != expected_size); // interrupt sould corrupt file or it shouldn't even exist
    }
    loop.quit();
    dfm->deleteLater();
}


void DownloadFileManagerTest::testInstance_data(){
    createTestData();
}

void DownloadFileManagerTest::testInteruptFunction_data(){
    createTestData();
}


void DownloadFileManagerTest::createTestData() {
    QTest::addColumn<QString>("kurjun_file_id");
    QTest::addColumn<QString>("dst_file");
    QTest::addColumn<int>("expected_size");
    QTest::addColumn<int>("time_to_download");


    /// Test #1
    std::vector<CGorjunFileInfo> fi =
    CRestWorker::Instance()->get_gorjun_file_info(p2p_kurjun_file_name());
    QString dst_file =
         QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0] + QDir::separator() + p2p_kurjun_file_name();
    int time_to_download = 4000; // let's give 4 seconds to download p2p
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
    time_to_download = 2000; // 2 seconds to download tray
    QTest::newRow("tray kurjun file with location in temp directory")
        << fi.begin()->id()
        << dst_file
        << fi.begin()->size()
        << time_to_download;
}
