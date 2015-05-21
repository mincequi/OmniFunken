#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T16:29:46
#
#-------------------------------------------------

QT       += testlib network

QT       -= gui

TARGET = tst_rtptest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../src

SOURCES += tst_rtptest.cpp \
    ../../src/rtp/rtpbuffer.cpp \
    ../../src/rtp/rtpbufferalt.cpp \
    ../../src/rtp/rtpheader.cpp \
    ../../src/util.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../src/rtp/rtpbuffer.h \
    ../../src/rtp/rtpbufferalt.h \
    ../../src/rtp/rtpheader.h \
    ../../src/rtp/rtppacket.h \
    ../../src/util.h
