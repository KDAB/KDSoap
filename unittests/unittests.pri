#    Note: KDSOAP_PATH is set in the calling .pro file
include (../examples/examples.pri)

# Unittests shouldn't be in ../bin, it breaks 'nmake test' on Windows and makes things more difficult for developing on linux
DESTDIR=.

CONFIG += qtestlib

INCLUDEPATH += $$KDSOAP_PATH/testtools
DEPENDPATH += $$KDSOAP_PATH/testtools

DEBUG_SUFFIX=""
CONFIG(debug, debug|release):!unix: DEBUG_SUFFIX = d
LIBS += -L../lib -ltesttools$$DEBUG_SUFFIX

# qtest.h in 4.5 is not QT_NO_CAST_FROM_BYTEARRAY-clean
contains( $$list($$[QT_VERSION]), 4.5.* ):DEFINES -= QT_NO_CAST_FROM_BYTEARRAY
