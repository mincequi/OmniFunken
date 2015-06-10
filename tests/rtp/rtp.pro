#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T16:29:46
#
#-------------------------------------------------

QT       += testlib network
QT       -= gui

QMAKE_CXXFLAGS += -std=c++0x

TARGET = tst_rtptest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../src

macx {
    INCLUDEPATH += "/usr/local/include/"
    LIBS += -L/usr/local/lib
}

LIBS += -lao

SOURCES += tst_rtptest.cpp \
    ../../src/rtp/rtpbuffer.cpp \
    ../../src/rtp/rtpheader.cpp \
    ../../src/util.cpp \
    ../../src/audioout/audioout_ao.cpp \
    ../../src/audioout/audiooutfactory.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../src/rtp/rtpbuffer.h \
    ../../src/rtp/rtpheader.h \
    ../../src/rtp/rtppacket.h \
    ../../src/util.h \
    ../../src/audioout/audioout_ao.h \
    ../../src/audioout/audiooutfactory.h
