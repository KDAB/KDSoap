TEMPLATE = lib
TARGET = wsdl
CONFIG += staticlib

SOURCES = \
   binding.cpp \
   bindingoperation.cpp \
   definitions.cpp \
   element.cpp \
   fault.cpp \
   import.cpp \
   message.cpp \
   operation.cpp \
   param.cpp \
   part.cpp \
   port.cpp \
   porttype.cpp \
   service.cpp \
   soapbinding.cpp \
   type.cpp \
   wsdl.cpp

QT -= gui
QT += xml
LIBS += kxmlcommon schema

INCLUDEPATH += $${TOP_SOURCE_DIR}/kdwsdl2cpp

include($${TOP_SOURCE_DIR}/variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII QBA_NO_CAST_TO_VOID QBA_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

