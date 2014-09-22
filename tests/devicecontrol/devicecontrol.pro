#-------------------------------------------------
#
# Project created by QtCreator 2014-09-22T19:26:25
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_devicecontroltest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += tst_devicecontroltest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../../src

SOURCES += ../../src/devicecontrol/devicecontrolfactory.cpp

HEADERS += ../../src/devicecontrol/devicecontrolabstract.h \
    ../../src/devicecontrol/devicecontrolfactory.h
