#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T16:35:29
#
#-------------------------------------------------

QT      += core network #serialport
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

LIBS += -lcrypto -lao

unix:!macx {
    LIBS += -ldns_sd -lasound
}

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

SOURCES += main.cpp \
    player.cpp \
    rtspmessage.cpp \
    rtspserver.cpp \
    rtpreceiver.cpp \
    alac.c \
    ratecontrol.cpp \
    rtpbufferalt.cpp \
    omnifunken.cpp \
    rtpbuffer.cpp \
    daemon.c \
    util.cpp \
    devicecontrol/devicecontrolfactory.cpp \
    audioout/audiooutfactory.cpp \
    audioout/audioout_ao.cpp \
    zeroconf/zeroconf_dns_sd.cpp

unix:!macx {
    SOURCES += audioout/audioout_alsa.cpp
}


HEADERS += \
    player.h \
    rtspmessage.h \
    rtspserver.h \
    rtpreceiver.h \
    alac.h \
    ratecontrol.h \
    rtpbufferalt.h \
    daemon.h \
    omnifunken.h \
    rtppacket.h \
    rtpbuffer.h \
    util.h \
    devicecontrol/devicecontrolfactory.h \
    devicecontrol/devicecontrolabstract.h \
    audioout/audioout_abstract.h \
    audioout/audioout_ao.h \
    audioout/audiooutfactory.h \
    zeroconf/zeroconf.h \
    zeroconf/zeroconf_dns_sd.h \
    airtunes/airtunes.h \
    airplay/airplay.h

unix:!macx {
    HEADERS += audioout/audioout_alsa.h
}


OTHER_FILES += \
    etc/init.d/omnifunken \
    etc/omnifunken.conf
