QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
QT += testlib

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += main
INCLUDEPATH += unit_tests
INCLUDEPATH += "../"




SOURCES += main/main.cpp \
    main/tester.cpp


HEADERS += \
    main/tester.h
