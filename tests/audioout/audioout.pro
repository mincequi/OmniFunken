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


QMAKE_CXXFLAGS += -std=c++11

macx {
    INCLUDEPATH += "/usr/local/Cellar/libao/1.2.0/include/"
    LIBS += -L/usr/local/Cellar/libao/1.2.0/lib
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
