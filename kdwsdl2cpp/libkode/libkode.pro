TEMPLATE = lib
CONFIG += staticlib
TARGET = kode
SOURCES += \
   code.cpp \
   enum.cpp \
   style.cpp \
   printer.cpp \
   license.cpp \
   file.cpp \
   class.cpp \
   function.cpp \
   variable.cpp \
   membervariable.cpp \
   typedef.cpp \
   statemachine.cpp

QT -= gui

INCLUDEPATH += $${TOP_SOURCE_DIR}/kdwsdl2cpp

include($${TOP_SOURCE_DIR}/variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII QBA_NO_CAST_TO_VOID QBA_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

