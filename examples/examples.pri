# This file is included by all of the examples' *.pro files.

KDSOAPLIB = kdsoap
win32* {
    CONFIG(debug, debug|release) {
        KDSOAPLIB = $${KDSOAPLIB}d
    }
}

# Adjust the paths and LIBS according to KDSOAP_PATH.
#    Note: KDSOAP_PATH is set in the calling .pro file,
#          before examples.pri is included
INCLUDEPATH += \
            $$KDSOAP_PATH/src \
            $$KDSOAP_PATH/include \
            $$KDSOAP_PATH/examples/tools
DEPENDPATH += \
            $$KDSOAP_PATH/src \
            $$KDSOAP_PATH/examples/tools
LIBS        += -L$$KDSOAP_PATH/lib -l$$KDSOAPLIB
!isEmpty(QMAKE_RPATH):LIBS += $$QMAKE_RPATH$$KDSOAP_PATH/lib

include(../variables.pri)
DEFINES -= QT_NO_CAST_FROM_ASCII

include(../kdwsdl2cpp.pri)
