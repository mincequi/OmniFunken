#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T16:35:29
#
#-------------------------------------------------

QT      += core network serialport
QT      -= gui

TARGET = omnifunken
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

CONFIG += debug_and_release


macx {
    INCLUDEPATH += "/usr/local/Cellar/libao/1.2.0/include/"
    LIBS += -L/usr/local/Cellar/libao/1.2.0/lib
}

# Needed for qt serial port
INCLUDEPATH += "/usr/include/qt5"

LIBS += -lcrypto -lao

unix:!macx {
    LIBS += -ldns_sd -lasound -ludev
}

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

SOURCES += main.cpp \
    player.cpp \
    alac.c \
    ratecontrol.cpp \
    omnifunken.cpp \
    daemon.c \
    util.cpp \
    devicecontrol/devicecontrolfactory.cpp \
    audioout/audiooutfactory.cpp \
    audioout/audioout_ao.cpp \
    zeroconf/zeroconf_dns_sd.cpp \
    rtp/rtpbuffer.cpp \
    rtp/rtpbufferalt.cpp \
    rtp/rtpreceiver.cpp \
    rtsp/rtspmessage.cpp \
    rtsp/rtspserver.cpp \
    devicecontrol/devicecontrolrs232.cpp \
    devicecontrol/devicewatcher.cpp \
    signalhandler.cpp \
    service/serviceconfig.cpp \
    service/service.cpp \
    airtunes/airtunesserviceconfig.cpp \
    core/core.cpp

unix:!macx {
    SOURCES += audioout/audioout_alsa.cpp
}


HEADERS += \
    player.h \
    alac.h \
    ratecontrol.h \
    daemon.h \
    omnifunken.h \
    util.h \
    devicecontrol/devicecontrolfactory.h \
    devicecontrol/devicecontrolabstract.h \
    devicecontrol/devicecontrolrs232.h \
    devicecontrol/devicewatcher.h \
    audioout/audioout_abstract.h \
    audioout/audioout_ao.h \
    audioout/audiooutfactory.h \
    zeroconf/zeroconf.h \
    zeroconf/zeroconf_dns_sd.h \
    airplay/airplay.h \
    rtp/rtppacket.h \
    rtp/rtpbuffer.h \
    rtp/rtpbufferalt.h \
    airtunes/airtunesconstants.h \
    rtp/rtpreceiver.h \
    rtsp/rtspserver.h \
    rtsp/rtspmessage.h \
    signalhandler.h \
    audiofilter/audiofilterabstract.h \
    service/service.h \
    service/serviceconfig.h \
    airtunes/airtunesserviceconfig.h \
    core/core.h

unix:!macx {
    HEADERS += audioout/audioout_alsa.h
}

