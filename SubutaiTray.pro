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

QMAKE_CXXFLAGS += -Wall

INCLUDEPATH += commons/include
INCLUDEPATH += hub/include
INCLUDEPATH += vbox/include
INCLUDEPATH += libssh2/include

SOURCES += \
    main.cpp \
    commons/src/EventLoop.cpp \
    commons/src/IFunctor.cpp \
    commons/src/ApplicationLog.cpp \
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
    commons/src/MRE_Pthread.cpp \
    hub/src/NotificationObserver.cpp

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
    commons/include/ApplicationLog.h \
    commons/include/EventLoop.h \
    commons/include/EventLoopException.h \
    commons/include/EventLoopExceptionInfo.h \
    commons/include/FileWrapper.h \
    commons/include/FunctorWithoutResult.h \
    commons/include/FunctorWithResult.h \
    commons/include/IFunctor.h \
    commons/include/InternalCriticalSection.h \
    commons/include/IRunnable.h \
    commons/include/Locker.h \
    commons/include/MRE_Wrapper.h \
    commons/include/ThreadWrapper.h \
    commons/include/Commons.h \
    commons/include/MRE_Windows.h \
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
    commons/include/MRE_Pthread.h \
    hub/include/NotificationObserver.h

FORMS    += \
    hub/forms/DlgLogin.ui \
    hub/forms/DlgSettings.ui \
    hub/forms/TrayControlWindow.ui \
    hub/forms/DlgAbout.ui \
    hub/forms/DlgGenerateSshKey.ui \
    hub/forms/DlgNotifications.ui

RESOURCES += \
    resources/resources.qrc

GIT_VERSION = $$system(git describe)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

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
  LIBS += -L/usr/local/lib/ -lssh2
#  USE WITH CROSS COMPILATION
#  LIBS += -L$$PWD/libssh2/lib -lssh2
#  QMAKE_CXXFLAGS += -fshort-wchar -stdlib=libc++ -std=c++11
#  QMAKE_LFLAGS += -stdlib=libc++
}
#////////////////////////////////////////////////////////////////////////////

win32: {
  DEFINES += CURRENT_OS=OS_WIN
  DEFINES += RT_OS_WINDOWS
  LIBS += -lws2_32 -lpthread -L$$PWD/libssh2/lib/win32 -lssh2
  QMAKE_LFLAGS += -static-libstdc++ -static-libgcc
}
#////////////////////////////////////////////////////////////////////////////
