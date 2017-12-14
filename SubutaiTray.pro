#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T12:15:31
#
#-------------------------------------------------

QT       += core gui network websockets
CONFIG   += c++11



greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets

TARGET = SubutaiTray
TEMPLATE = app

INCLUDEPATH += commons/include
INCLUDEPATH += hub/include
INCLUDEPATH += vbox/include
INCLUDEPATH += libssh2/include

SOURCES += \
    main.cpp \
    commons/src/InternalCriticalSection.cpp \
    commons/src/Commons.cpp \
    hub/src/RestWorker.cpp \
    hub/src/DlgLogin.cpp \
    hub/src/SettingsManager.cpp \
    hub/src/DlgSettings.cpp \
    hub/src/TrayControlWindow.cpp \
    hub/src/SystemCallWrapper.cpp \
    hub/src/TrayWebSocketServer.cpp \
    hub/src/HubController.cpp \
    hub/src/DlgAbout.cpp \
    hub/src/DownloadFileManager.cpp \
    hub/src/updater/ExecutableUpdater.cpp \
    hub/src/DlgGenerateSshKey.cpp \
    hub/src/updater/HubComponentsUpdater.cpp \
    hub/src/updater/UpdaterComponentRH.cpp \
    hub/src/updater/UpdaterComponentP2P.cpp \
    hub/src/updater/UpdaterComponentTray.cpp \
    hub/src/updater/IUpdaterComponent.cpp \
    libssh2/src/LibsshController.cpp \
    hub/src/updater/UpdaterComponentRHManagement.cpp \
    hub/src/SshKeysController.cpp \
    commons/src/OsBranchConsts.cpp \
    hub/src/SsdpController.cpp \
    hub/src/RhController.cpp \
    hub/src/NotificationLogger.cpp \
    hub/src/DlgNotifications.cpp \
    vbox/src/VBoxManager.cpp \
    vbox/src/VirtualMachine.cpp \
    hub/src/NotificationObserver.cpp \
    hub/src/DlgNotificationsModel.cpp \
    hub/src/DlgNotification.cpp \
    commons/src/Logger.cpp \
    commons/src/LanguageController.cpp \
    hub/src/DlgEnvironment.cpp \
    hub/src/P2PController.cpp




HEADERS  += \
    hub/include/RestWorker.h \
    hub/include/DlgLogin.h \
    hub/include/SettingsManager.h \
    hub/include/DlgSettings.h \
    hub/include/TrayControlWindow.h \
    hub/include/SystemCallWrapper.h \
    hub/include/TrayWebSocketServer.h \
    hub/include/HubController.h \
    hub/include/DlgAbout.h \
    hub/include/RestContainers.h \
    hub/include/DownloadFileManager.h \
    hub/include/updater/ExecutableUpdater.h \
    hub/include/DlgGenerateSshKey.h \
    hub/include/updater/HubComponentsUpdater.h \
    hub/include/updater/IUpdaterComponent.h \
    hub/include/updater/UpdaterComponentRH.h \
    hub/include/updater/UpdaterComponentP2P.h \
    hub/include/updater/UpdaterComponentTray.h \
    commons/include/InternalCriticalSection.h \
    commons/include/Locker.h \
    commons/include/Commons.h \
    libssh2/include/LibsshController.h \
    hub/include/updater/UpdaterComponentRHManagement.h \
    hub/include/SshKeysController.h \
    commons/include/OsBranchConsts.h \
    hub/include/SsdpController.h \
    hub/include/RhController.h \
    hub/include/NotificationLogger.h \
    hub/include/DlgNotifications.h \
    vbox/include/VBoxManager.h \
    vbox/include/VirtualMachine.h \
    hub/include/NotificationObserver.h \
    hub/include/DlgNotificationsModel.h \
    hub/include/DlgNotification.h \
    commons/include/Logger.h \
    commons/include/LanguageController.h \
    hub/include/DlgEnvironment.h \
    hub/include/P2PController.h

TRANSLATIONS = SubutaiTray_en_US.ts \
               SubutaiTray_ru_RU.ts \
               SubutaiTray_pt_BR.ts

FORMS    += \
    hub/forms/DlgLogin.ui \
    hub/forms/DlgSettings.ui \
    hub/forms/TrayControlWindow.ui \
    hub/forms/DlgAbout.ui \
    hub/forms/DlgGenerateSshKey.ui \
    hub/forms/DlgNotifications.ui \
    hub/forms/DlgNotification.ui \
    hub/forms/DlgEnvironment.ui \

RESOURCES += \
    resources/resources.qrc

unix: {
  TRAY_VERSION = $$system(cat version)
}
win32: {
  TRAY_VERSION = $$system(type version)
}

DEFINES += QT_MESSAGELOGCONTEXT

DEFINES += TRAY_VERSION=\\\"$$TRAY_VERSION\\\"
GIT_BRANCH_STR = $$system(git rev-parse --abbrev-ref HEAD)
DEFINES += GIT_BRANCH=\\\"$$GIT_BRANCH_STR\\\"

GBV=BT_DEV
equals(GIT_BRANCH_STR, "master") {
  GBV=BT_MASTER
}

equals(GIT_BRANCH_STR, "HEAD") {
  GBV=BT_PROD
}

equals(GIT_BRANCH_STR, "head") {
  GBV=BT_PROD
}

equals(GIT_BRANCH_STR, "dev") {
  GBV=BT_DEV
}
DEFINES += CURRENT_BRANCH=$$GBV
#////////////////////////////////////////////////////////////////////////////

unix:!macx {
  QMAKE_CXXFLAGS += -fshort-wchar
  DEFINES += RT_OS_LINUX
  DEFINES += CURRENT_OS=OS_LINUX
  LIBS += -ldl -lpthread -lssh2
  QMAKE_RPATHDIR += ../lib
}
#////////////////////////////////////////////////////////////////////////////

macx: {
  DEFINES += RT_OS_DARWIN
  DEFINES += CURRENT_OS=OS_MAC
  QMAKE_LFLAGS += -F /System/Library/Frameworks/CoreFoundation.framework/
  LIBS += -framework CoreFoundation
  LIBS += -ldl -lpthread
  ICON = $$PWD/resources/tray_logo.icns
  QMAKE_INFO_PLIST = $$PWD/Info.plist
  LIBS += -L/tmp/tray-static -L/usr/local/lib/ -lssh2
#  USE WITH CROSS COMPILATION
#  LIBS += -L$$PWD/libssh2/lib -lssh2
#  QMAKE_CXXFLAGS += -fshort-wchar -stdlib=libc++ -std=c++11
#  QMAKE_LFLAGS += -stdlib=libc++
}
#////////////////////////////////////////////////////////////////////////////

win32: {
  DEFINES += CURRENT_OS=OS_WIN
  DEFINES += RT_OS_WINDOWS
  LIBS += -lws2_32 -L$$PWD/libssh2/lib/win32 -llibssh2
  RC_FILE = tray.rc
#  LIBS += -lpthread
# QMAKE_LFLAGS += -static-libstdc++ -static-libgcc
}
#////////////////////////////////////////////////////////////////////////////

tests {
    message(Test build)
    QT += testlib
    TARGET = TestingTray

    LIBS += -lgcov

    #QMAKE_CXXFLAGS += --coverage
    #QMAKE_LFLAGS += --coverage
    QMAKE_CXXFLAGS += -g -fprofile-arcs -ftest-coverage -O0
    QMAKE_LFLAGS += -g -fprofile-arcs -ftest-coverage  -O0

    SOURCES -= main.cpp

    HEADERS += tests/CCommonsTest.h \
        tests/Tester.h \
        tests/LanguageControllerTest.h \
        tests/LoggerTest.h \
        tests/OsBranchConstsTest.h \
        tests/TrayWebSocketServerTest.h \
        tests/SystemCallWrapperTest.h \
        tests/DlgNotificationsModelTest.h \
        tests/NotificationObserverTest.h \
        tests/DlgSettingsTest.h \
        tests/NotificationLoggerTest.h \
        tests/SettingsManagerTest.h \
        tests/RhControllerTest.h \
        tests/HubControlllerTest.h \
        tests/DownloadFileManagerTest.h \
        tests/RestWorkerTest.h

    SOURCES += tests/main.cpp \
        tests/CCommonsTest.cpp \
        tests/Tester.cpp \
        tests/LanguageControllerTest.cpp \
        tests/LoggerTest.cpp \
        tests/OsBranchConstsTest.cpp \
        tests/TrayWebSocketServerTest.cpp \
        tests/SystemCallWrapperTest.cpp \
        tests/DlgNotificationsModelTest.cpp \
        tests/NotificationObserverTest.cpp \
        tests/DlgSettingsTest.cpp \
        tests/NotificationLoggerTest.cpp \
        tests/SettingsManagerTest.cpp \
        tests/RhControllerTest.cpp \
        tests/HubControlllerTest.cpp \
        tests/DownloadFileManagerTest.cpp \
        tests/RestWorkerTest.cpp
} else {
    message(Normal build)
}
