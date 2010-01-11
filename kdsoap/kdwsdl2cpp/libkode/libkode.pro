TEMPLATE = lib
CONFIG += static
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

INCLUDEPATH += ..

include(../../variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII QBA_NO_CAST_TO_VOID QBA_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

