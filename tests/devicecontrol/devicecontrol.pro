#-------------------------------------------------
#
# Project created by QtCreator 2014-09-22T19:26:25
#
#-------------------------------------------------

QT       += testlib serialport

QT       -= gui

TARGET = tst_devicecontroltest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += tst_devicecontroltest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../../src \
	/usr/include/qt5

LIBS += -ludev

SOURCES += ../../src/devicecontrol/devicecontrolfactory.cpp \
    ../../src/devicecontrol/devicecontrolrs232.cpp \
    ../../src/devicecontrol/devicewatcher.cpp

HEADERS += ../../src/devicecontrol/devicecontrolabstract.h \
    ../../src/devicecontrol/devicecontrolfactory.h \
    ../../src/devicecontrol/devicecontrolrs232.h \
    ../../src/devicecontrol/devicewatcher.h
