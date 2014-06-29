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

LIBS += -lcrypto -ldns_sd #-lssl #-lcrypto -lz

SOURCES += main.cpp \
    player.cpp \
    audio_out_alsa.cpp \
    audio_buffer.cpp \
    zeroconf_dns_sd.cpp \
    rtspmessage.cpp \
    rtspserver.cpp \
    rtpreceiver.cpp

HEADERS += \
    player.h \
    audio_out_alsa.h \
    audio_buffer.h \
    audio_out.h \
    zeroconf.h \
    zeroconf_dns_sd.h \
    rtspmessage.h \
    rtspserver.h \
    rtpreceiver.h
