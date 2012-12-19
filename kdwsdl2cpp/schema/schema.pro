option(host_build) # Qt5 option, ignore warning with Qt4
TEMPLATE = lib
TARGET = xmlschema
CONFIG += staticlib

SOURCES = \
   annotation.cpp \
   attribute.cpp \
   complextype.cpp \
   element.cpp \
   parser.cpp \
   simpletype.cpp \
   types.cpp \
   xmlelement.cpp \
   xsdtype.cpp \
   attributegroup.cpp \
   compositor.cpp

QT -= gui
QT += xml

INCLUDEPATH += $${TOP_SOURCE_DIR}/kdwsdl2cpp

include($${TOP_SOURCE_DIR}/variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII QBA_NO_CAST_TO_VOID QBA_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

