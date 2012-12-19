#    Note: KDSOAP_PATH is set in the calling .pro file
include ($${TOP_SOURCE_DIR}/examples/examples.pri)

# Unittests shouldn't be in ../bin, it breaks 'nmake test' on Windows and makes things more difficult for developing on linux
DESTDIR=.

QT += testlib
QT += xml

INCLUDEPATH += $${TOP_SOURCE_DIR}/testtools
DEPENDPATH += $${TOP_SOURCE_DIR}/testtools

DEBUG_SUFFIX=""
CONFIG(debug, debug|release):!unix: DEBUG_SUFFIX = d
LIBS += -L$${TOP_BUILD_DIR}/lib -ltesttools$$DEBUG_SUFFIX
unix:PRE_TARGET_DEPS = $${TOP_BUILD_DIR}/lib/libtesttools$$DEBUG_SUFFIX.a

# qtest.h in 4.5 is not QT_NO_CAST_FROM_BYTEARRAY-clean
contains( $$list($$[QT_VERSION]), 4.5.* ):DEFINES -= QT_NO_CAST_FROM_BYTEARRAY
