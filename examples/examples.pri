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
