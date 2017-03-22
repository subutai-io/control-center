#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T12:15:31
#
#-------------------------------------------------

QT       += core gui network websockets sql
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets

TARGET = SubutaiTray
TEMPLATE = app

INCLUDEPATH += commons/include
INCLUDEPATH += hub/include
INCLUDEPATH += vbox/include
INCLUDEPATH += vbox/sdk_includes
INCLUDEPATH += vbox/sdk_includes/xpcom
INCLUDEPATH += vbox/sdk_includes/nsprpub
INCLUDEPATH += vbox/sdk_includes/string
INCLUDEPATH += vbox/sdk_includes/ipcd
INCLUDEPATH += libssh2/include

SOURCES += \
    main.cpp \
    vbox/src/IVBoxManager.cpp \    
    commons/src/EventLoop.cpp \
    commons/src/IFunctor.cpp \
    commons/src/ApplicationLog.cpp \
    commons/src/InternalCriticalSection.cpp \
    commons/src/MRE_Linux.cpp \
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
    hub/src/SsdpController.cpp

HEADERS  += \
    hub/include/RestWorker.h \
    hub/include/DlgLogin.h \
    hub/include/SettingsManager.h \
    hub/include/DlgSettings.h \
    hub/include/TrayControlWindow.h \
    hub/include/SystemCallWrapper.h \
    hub/include/NotifiactionObserver.h \
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
    vbox/include/VBoxCommons.h \
    vbox/include/IVBoxManager.h \
    vbox/include/IVirtualMachine.h \
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
    commons/include/MRE_Linux.h \
    commons/include/MRE_Wrapper.h \
    commons/include/ThreadWrapper.h \
    commons/include/Commons.h \
    commons/include/MRE_Windows.h \
    libssh2/include/LibsshController.h \
    hub/include/updater/UpdaterComponentRHManagement.h \
    hub/include/SshKeysController.h \
    commons/include/OsBranchConsts.h \
    hub/include/SsdpController.h

FORMS    += \
    hub/forms/DlgLogin.ui \
    hub/forms/DlgSettings.ui \
    hub/forms/TrayControlWindow.ui \
    hub/forms/DlgAbout.ui \
    hub/forms/DlgGenerateSshKey.ui \

RESOURCES += \
    resources/resources.qrc

GIT_VERSION = $$system(git describe)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

GIT_BRANCH_STR = $$system(git rev-parse --abbrev-ref HEAD)
DEFINES += GIT_BRANCH=\\\"$$GIT_BRANCH_STR\\\"

GBV = BT_DEV
equals(GIT_BRANCH_STR, "master") {
  GBV=BT_MASTER
}

equals(GIT_BRANCH_STR, "stage") {
  GBV=BT_STAGE
}

equals(GIT_BRANCH_STR, "dev") {
  GBV=BT_DEV
}
DEFINES += CURRENT_BRANCH=$$GBV
#////////////////////////////////////////////////////////////////////////////

unix:!macx {
  QMAKE_CXXFLAGS += -fshort-wchar
  DEFINES += VBOX_WITH_XPCOM_NAMESPACE_CLEANUP RT_OS_LINUX
  DEFINES += VBOX_WITH_XPCOM IN_RING3
  DEFINES += CURRENT_OS=OS_LINUX

  HEADERS +=  vbox/include/VBoxManagerLinux.h \
              vbox/include/VirtualMachineLinux.h

  SOURCES +=  vbox/src/VBoxManagerLinux.cpp \
              vbox/src/VirtualMachineLinux.cpp

  LIBS += /usr/lib/virtualbox/VBoxXPCOM.so
  QMAKE_RPATHDIR += /usr/lib/virtualbox/
  LIBS += -ldl -lpthread
  QMAKE_RPATHDIR += ../lib

  LIBS += -lssh2
}
#////////////////////////////////////////////////////////////////////////////

macx: {
  QMAKE_CXXFLAGS += -fshort-wchar
#  seems like clang compiler can't resolve some functions presented in nsXPCOM.h
#  but it uses VBoxXPCOM.dylib
#  DEFINES += VBOX_WITH_XPCOM_NAMESPACE_CLEANUP

  DEFINES += RT_OS_DARWIN
  DEFINES += VBOX_WITH_XPCOM
  DEFINES += IN_RING3
  DEFINES += CURRENT_OS=OS_MAC

  HEADERS +=  vbox/include/VBoxManagerLinux.h \
              vbox/include/VirtualMachineLinux.h
  SOURCES +=  vbox/src/VBoxManagerLinux.cpp \
              vbox/src/VirtualMachineLinux.cpp

  QMAKE_LFLAGS += -F /System/Library/Frameworks/CoreFoundation.framework/
  LIBS += /Applications/VirtualBox.app/Contents/MacOS/VBoxXPCOM.dylib
  LIBS += -framework CoreFoundation
  LIBS += -ldl -lpthread
  ICON = $$PWD/resources/tray_logo.icns
  QMAKE_INFO_PLIST = $$PWD/Info.plist

  LIBS += -L/usr/local/lib/ -lssh2
}
#////////////////////////////////////////////////////////////////////////////

win32: {
  DEFINES += RT_OS_WINDOWS IN_RING3
  LIBS += Ole32.lib Rpcrt4.lib
  DEFINES += CURRENT_OS=OS_WIN

  INCLUDEPATH += vbox/mscom/include

  SOURCES +=  vbox/mscom/lib/VirtualBox_i.c \
              vbox/src/VirtualMachineWin.cpp \
              vbox/src/VBoxManagerWin.cpp

  HEADERS +=  vbox/include/VirtualMachineWin.h \
              vbox/include/VBoxManagerWin.h

  LIBS += ws2_32.lib
  LIBS += $$PWD/libssh2/lib/win32/libssh2.lib
  LIBS += $$PWD/libssh2/lib/win32/libssh2.exp
}
#////////////////////////////////////////////////////////////////////////////
