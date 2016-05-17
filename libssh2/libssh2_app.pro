TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


unix:!macx {
  LIBS += -L$$PWD/lib/linux/ -lssh2
}

macx {
  LIBS += -L/usr/local/lib/ -lssh2
}

win32 {
  LIBS += ws2_32.lib
  LIBS += $$PWD/lib/win32/libssh2.lib
  LIBS += $$PWD/lib/win32/libssh2.exp

#  Code below (commented) doesn't work. Have no idea why. If you know - send info to lezh1k.vohrer@gmail.com
#  You can see error in compile output

#  copy_libssh2_dll.commands = $(COPY_FILE) \"$$PWD/lib/win32/libssh2.dll\" \"$$OUT_PWD\"
#  first.depends = $(first) copy_libssh2_dll
#  export(first.depends)
#  export(copy_libssh2_dll.commands)
#  QMAKE_EXTRA_TARGETS += first copy_libssh2_dll
}


INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

DEFINES += LOG_STD_OUT

SOURCES += main.cpp
HEADERS += \
    UpdateErrors.h
