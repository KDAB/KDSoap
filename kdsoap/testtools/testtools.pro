# Private library used by unittests

KDSOAP_PATH = ..
TEMPLATE = lib
CONFIG += staticlib
TARGET = testtools
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui

QT += network

# for QDomDocument in httpserver_p.cpp
QT += xml

# Workaround for visual studio integration
DESTDIR = ../lib
win32:DLLDESTDIR = ../bin

include(../variables.pri)
# To link to KDSoap
include(../examples/examples.pri)

SOURCES = httpserver_p.cpp
HEADERS = httpserver_p.h

