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
    audiooutfactory.cpp \
    audioout_ao.cpp

unix:!macx {
    SOURCES += audioout_alsa.cpp
}

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
    rtppacket.h \
    rtpbuffer.h \
    audioout_abstract.h \
    audiooutfactory.h \
    audioout_ao.h \
    devicecontrolabstract.h

unix:!macx {
    HEADERS += audioout_alsa.h
}

OTHER_FILES += \
    etc/init.d/omnifunken \
    etc/omnifunken.conf
