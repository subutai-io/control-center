TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


unix:!macx {
  LIBS += -L$$PWD/lib/linux/ -lssh2
}

macx {
  LIBS += -L/usr/local/opt/libssh2/lib/ -lssh2
}


INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

DEFINES += LOG_STD_OUT

SOURCES += main.cpp
HEADERS += \
    UpdateErrors.h
