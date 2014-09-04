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

INCLUDEPATH += ../../

SOURCES += ../../audioout/audiooutfactory.cpp \
    ../../audioout/audioout_ao.cpp
unix:!macx {
    SOURCES += ../../audioout/audioout_alsa.cpp
}

HEADERS += ../../audioout/audiooutfactory.h \
    ../../audioout/audioout_abstract.h \
    ../../audioout/audioout_ao.h
unix:!macx {
    HEADERS += ../../audioout/audioout_alsa.h
}
