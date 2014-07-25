#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T16:35:29
#
#-------------------------------------------------

QT       += core network gui

TARGET = OmniFunken
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

macx {
    INCLUDEPATH += "/usr/local/Cellar/libao/1.2.0/include/"
    LIBS += -L/usr/local/Cellar/libao/1.2.0/lib
}

LIBS += -lcrypto -lao #-ldns_sd #-lssl #-lcrypto -lz



SOURCES += main.cpp \
    player.cpp \
    audio_out_alsa.cpp \
    zeroconf_dns_sd.cpp \
    rtspmessage.cpp \
    rtspserver.cpp \
    rtpreceiver.cpp \
    alac.c \
    rtpbuffer.cpp \
    ratecontrol.cpp \
    rtpbufferalt.cpp \
    daemon.cpp \
    omnifunken.cpp

HEADERS += \
    player.h \
    audio_out_alsa.h \
    audio_out.h \
    zeroconf.h \
    zeroconf_dns_sd.h \
    rtspmessage.h \
    rtspserver.h \
    rtpreceiver.h \
    alac.h \
    rtpbuffer.h \
    ratecontrol.h \
    rtpbufferalt.h \
    daemon.h \
    omnifunken.h

OTHER_FILES += \
    omnifunken.conf
