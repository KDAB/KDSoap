## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

# Private library used by unittests

TEMPLATE = lib
CONFIG += staticlib
TARGET = testtools
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui

QT += network

# for QDomDocument in httpserver_p.cpp
QT += xml

# Workaround for visual studio integration
DESTDIR = $${TOP_BUILD_DIR}/lib
win32:DLLDESTDIR = $${TOP_BUILD_DIR}/bin

include($${TOP_SOURCE_DIR}/variables.pri)
# To link to KDSoap
include($${TOP_SOURCE_DIR}/examples/examples.pri)

SOURCES = httpserver_p.cpp
HEADERS = httpserver_p.h

RESOURCES += testtools.qrc

