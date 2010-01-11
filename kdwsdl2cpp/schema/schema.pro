TEMPLATE = lib
TARGET = xmlschema
CONFIG += static

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

INCLUDEPATH += ..

include(../../variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII QBA_NO_CAST_TO_VOID QBA_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

