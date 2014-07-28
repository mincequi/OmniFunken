#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T16:35:29
#
#-------------------------------------------------

QT      += core network
QT      -= gui

TARGET = omnifunken
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

macx {
    INCLUDEPATH += "/usr/local/Cellar/libao/1.2.0/include/"
    LIBS += -L/usr/local/Cellar/libao/1.2.0/lib
}

LIBS += -lcrypto -lao #-ldns_sd #-lssl #-lcrypto -lz

SOURCES += main.cpp \
    player.cpp \
    zeroconf_dns_sd.cpp \
    rtspmessage.cpp \
    rtspserver.cpp \
    rtpreceiver.cpp \
    alac.c \
    ratecontrol.cpp \
    rtpbufferalt.cpp \
    omnifunken.cpp \
    rtpbuffer.cpp \
    daemon.c \
    audioout_alsa.cpp \
    audiooutfactory.cpp \
    audioout_ao.cpp

HEADERS += \
    player.h \
    zeroconf.h \
    zeroconf_dns_sd.h \
    rtspmessage.h \
    rtspserver.h \
    rtpreceiver.h \
    alac.h \
    ratecontrol.h \
    rtpbufferalt.h \
    daemon.h \
    omnifunken.h \
    rtpbuffer.h \
    audioout_abstract.h \
    audioout_alsa.h \
    audiooutfactory.h \
    audioout_ao.h

OTHER_FILES += \
    etc/init.d/omnifunken \
    etc/omnifunken.conf
