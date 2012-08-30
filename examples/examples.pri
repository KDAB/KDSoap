# This file is included by all of the examples' *.pro files.

# KDSOAPLIB is defined by the toplevel kdsoap.pro and stored into .qmake.cache
isEmpty(KDSOAPLIB): error("KDSOAPLIB is empty. This should not happen, please check .qmake.cache at the toplevel")

INCLUDEPATH += \
            $${TOP_SOURCE_DIR}/src \
            $${TOP_SOURCE_DIR}/src/KDSoapClient \
            $${TOP_SOURCE_DIR}/src/KDSoapServer \
            $${TOP_SOURCE_DIR}/examples/tools
DEPENDPATH += \
            $${TOP_SOURCE_DIR}/src \
            $${TOP_SOURCE_DIR}/src/KDSoapClient \
            $${TOP_SOURCE_DIR}/src/KDSoapServer \
            $${TOP_SOURCE_DIR}/examples/tools
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPLIB
!isEmpty(QMAKE_LFLAGS_RPATH):LIBS += $$QMAKE_LFLAGS_RPATH$${TOP_BUILD_DIR}/lib

include($${TOP_SOURCE_DIR}/variables.pri)
DEFINES -= QT_NO_CAST_FROM_ASCII

include($${TOP_SOURCE_DIR}/kdwsdl2cpp.pri)

# Assume command-line by default
CONFIG += console
QT -= gui
macx:CONFIG -= app_bundle
