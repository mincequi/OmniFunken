#-------------------------------------------------
#
# Project created by QtCreator 2014-09-04T19:55:07
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_audioouttest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


QMAKE_CXXFLAGS += -std=c++0x

macx {
    INCLUDEPATH += "/usr/local/include/"
    LIBS += -L/usr/local/lib
}

LIBS += -lao
unix:!macx {
    LIBS += -lasound
}


SOURCES += tst_audioouttest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../../src

SOURCES += ../../src/audioout/audiooutfactory.cpp \
    ../../src/audioout/audioout_ao.cpp
unix:!macx {
    SOURCES += ../../src/audioout/audioout_alsa.cpp
}

HEADERS += ../../src/audioout/audiooutfactory.h \
    ../../src/audioout/audioout_abstract.h \
    ../../src/audioout/audioout_ao.h
unix:!macx {
    HEADERS += ../../src/audioout/audioout_alsa.h
}
