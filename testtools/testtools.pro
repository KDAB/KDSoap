# Private library used by unittests

TEMPLATE = lib
CONFIG += staticlib
TARGET = testtools
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui

QT += network

# for QDomDocument in httpserver_p.cpp
QT += xml

# Workaround for visual studio integration
DESTDIR = $${TOP_BUILD_DIR}/lib
win32:DLLDESTDIR = $${TOP_BUILD_DIR}/bin

include($${TOP_SOURCE_DIR}/variables.pri)
# To link to KDSoap
include($${TOP_SOURCE_DIR}/examples/examples.pri)

SOURCES = httpserver_p.cpp
HEADERS = httpserver_p.h

