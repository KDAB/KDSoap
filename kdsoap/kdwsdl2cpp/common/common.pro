TEMPLATE = lib
TARGET = xmlcommon
CONFIG += static
SOURCES = \
   fileprovider.cpp \
   messagehandler.cpp \
   nsmanager.cpp \
   parsercontext.cpp \
   qname.cpp

HEADERS = fileprovider.h

QT -= gui
QT += xml

INCLUDEPATH += .. ../libxmlcommon

include (../../variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII QBA_NO_CAST_TO_VOID QBA_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

