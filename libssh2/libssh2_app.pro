TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


unix:!macx {
  #should be installed libssh2. located in /usr/lib/libssh2
  LIBS += -lssh2
}

macx {
  LIBS += -L/usr/local/lib/ -lssh2
}

win32 {
  LIBS += ws2_32.lib
  LIBS += $$PWD/lib/win32/libssh2.lib
  LIBS += $$PWD/lib/win32/libssh2.exp
}


INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

DEFINES += LOG_STD_OUT

SOURCES += main.cpp
HEADERS += \
    LibsshErrors.h
