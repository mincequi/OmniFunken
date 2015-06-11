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
    daemon.c \
    util.cpp \
    devicecontrol/devicecontrolfactory.cpp \
    audioout/audiooutfactory.cpp \
    audioout/audioout_ao.cpp \
    devicecontrol/devicecontrolrs232.cpp \
    devicecontrol/devicewatcher.cpp \
    signalhandler.cpp \
    service/serviceconfig.cpp \
    service/service.cpp \
    airtunes/airtunesserviceconfig.cpp \
    core/core.cpp \
    rtp/rtpbuffer.cpp \
    rtp/rtpheader.cpp \
    rtp/rtpreceiver.cpp \
    #rtp/rtpretransmissionrequester.cpp \
    rtsp/rtspmessage.cpp \
    rtsp/rtspserver.cpp \
    #rtsp/rtspserver_threaded.cpp \
    rtsp/rtspsession.cpp \
    zeroconf/zeroconf_dns_sd.cpp \
    audioout/audioout_pipe.cpp

unix:!macx {
    SOURCES += audioout/audioout_alsa.cpp
}


HEADERS += \
    player.h \
    alac.h \
    daemon.h \
    signalhandler.h \
    util.h \
    airtunes/airtunesconstants.h \
    airtunes/airtunesserviceconfig.h \
    audiofilter/audiofilterabstract.h \
    audioout/audioout_abstract.h \
    audioout/audioout_ao.h \
    audioout/audiooutfactory.h \
    core/core.h \
    devicecontrol/devicecontrolfactory.h \
    devicecontrol/devicecontrolabstract.h \
    devicecontrol/devicecontrolrs232.h \
    devicecontrol/devicewatcher.h \
    rtp/rtpbuffer.h \
    rtp/rtpheader.h \
    rtp/rtppacket.h \
    rtp/rtpreceiver.h \
    #rtp/rtpretransmissionrequester.h \
    rtp/rtpstat.h \
    rtsp/rtspmessage.h \
    rtsp/rtspserver.h \
    #rtsp/rtspserver_threaded.h \
    rtsp/rtspsession.h \
    service/service.h \
    service/serviceconfig.h \
    global.h \
    zeroconf/zeroconf.h \
    zeroconf/zeroconf_dns_sd.h \
    audioout/audioout_pipe.h

unix:!macx {
    HEADERS += audioout/audioout_alsa.h
}

