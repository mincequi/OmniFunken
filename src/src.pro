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
    INCLUDEPATH += "/usr/local/include/"
    LIBS += -L/usr/local/lib
}

# Needed for qt serial port
INCLUDEPATH += "/usr/include/qt5"

LIBS += -lcrypto -lao -lboost_system

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
    devicecontrol/devicecontrolrs232.cpp \
    devicecontrol/devicewatcher.cpp \
    signalhandler.cpp \
    service/serviceconfig.cpp \
    service/service.cpp \
    airtunes/airtunesserviceconfig.cpp \
    core/core.cpp \
    rtp/rtpheader.cpp \
    #rtp/rtpbuffer.cpp \
    rtp/rtpbufferalt.cpp \
    #rtp/rtpreceiver.cpp \
    rtp/rtpreceiverboost.cpp \
    #rtp/rtpretransmissionrequester.cpp \
    rtsp/rtspmessage.cpp \
    rtsp/rtspserver.cpp \
    #rtsp/rtspserver_threaded.cpp \
    rtsp/rtspworker.cpp

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
    airplay/airplay.h \
    airtunes/airtunesconstants.h \
    devicecontrol/devicecontrolfactory.h \
    devicecontrol/devicecontrolabstract.h \
    devicecontrol/devicecontrolrs232.h \
    devicecontrol/devicewatcher.h \
    audioout/audioout_abstract.h \
    audioout/audioout_ao.h \
    audioout/audiooutfactory.h \
    rtp/rtppacket.h \
    #rtp/rtpbuffer.h \
    rtp/rtpbufferalt.h \
    #rtp/rtpreceiver.h \
    rtp/rtpreceiverboost.h \
    #rtp/rtpretransmissionrequester.h \
    rtp/rtpstat.h \
    rtsp/rtspserver.h \
    #rtsp/rtspserver_threaded.h \
    rtsp/rtspmessage.h \
    signalhandler.h \
    audiofilter/audiofilterabstract.h \
    service/service.h \
    service/serviceconfig.h \
    airtunes/airtunesserviceconfig.h \
    core/core.h \
    global.h \
    zeroconf/zeroconf.h \
    zeroconf/zeroconf_dns_sd.h \
    rtp/rtpheader.h \
    rtsp/rtspworker.h

unix:!macx {
    HEADERS += audioout/audioout_alsa.h
}

