## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2009-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

#    Note: KDSOAP_PATH is set in the calling .pro file
include ($${TOP_SOURCE_DIR}/examples/examples.pri)

# Unittests shouldn't be in ../bin, it breaks 'nmake test' on Windows and makes things more difficult for developing on linux
DESTDIR=.

QT += testlib
QT += xml

macx:CONFIG -= app_bundle
static:macx:QMAKE_LFLAGS += -Wl,-rpath,$$[QT_INSTALL_LIBS]

INCLUDEPATH += $${TOP_SOURCE_DIR}/testtools \
    $${TOP_SOURCE_DIR}/kdwsdl2cpp/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/src/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode/code_generation/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode/schema/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/wsdl/
DEPENDPATH += $${TOP_SOURCE_DIR}/testtools \
    $${TOP_SOURCE_DIR}/kdwsdl2cpp/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/src/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode/code_generation/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode/common/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode/schema/ $${TOP_SOURCE_DIR}/kdwsdl2cpp/wsdl/

DEBUG_SUFFIX=""
CONFIG(debug, debug|release):!unix: DEBUG_SUFFIX = d
LIBS += -L$${TOP_BUILD_DIR}/lib \
        -L$${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/code_generation \
        -L$${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/schema \
        -L$${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/common \
        -ltesttools$$DEBUG_SUFFIX
unix:PRE_TARGETDEPS += $${TOP_BUILD_DIR}/lib/libtesttools.a

KDWSDL2CPP_LIBS = -lkdwsdl2cpp_lib$$DEBUG_SUFFIX -lkode -lwsdl -lxmlschema -lxmlcommon

# qtest.h in 4.5 is not QT_NO_CAST_FROM_BYTEARRAY-clean
contains( $$list($$[QT_VERSION]), 4.5.* ):DEFINES -= QT_NO_CAST_FROM_BYTEARRAY
