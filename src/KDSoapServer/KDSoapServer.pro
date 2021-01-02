## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2011-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LicenseRef-KDAB-KDSoap-AGPL3-Modified OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

TEMPLATE = lib
TARGET = $${KDSOAPSERVERLIB}
QT -= gui

QT += network

# Workaround for visual studio integration
DESTDIR = $${TOP_BUILD_DIR}/lib
win32:DLLDESTDIR = $${TOP_BUILD_DIR}/bin

include($${TOP_SOURCE_DIR}/variables.pri)

# Only used by Mac frameworks.
# See the include subdir for standard header installation
# TODO: install these from include/ as well
INSTALLHEADERS = KDSoapServer.h \
                 KDSoapServerAuthInterface.h \
                 KDSoapServerRawXMLInterface.h \
                 KDSoapServerObjectInterface.h \
                 KDSoapServerGlobal.h \
                 KDSoapDelayedResponseHandle.h \
                 KDSoapServerCustomVerbRequestInterface.h

HEADERS = $$INSTALLHEADERS \
    KDSoapThreadPool.h \
    KDSoapServerSocket_p.h \
    KDSoapServerThread_p.h \
    KDSoapSocketList_p.h \

SOURCES = KDSoapServer.cpp \
    KDSoapThreadPool.cpp \
    KDSoapServerSocket.cpp \
    KDSoapServerThread.cpp \
    KDSoapSocketList.cpp \
    KDSoapServerAuthInterface.cpp \
    KDSoapServerRawXMLInterface.cpp \
    KDSoapServerObjectInterface.cpp \
    KDSoapDelayedResponseHandle.cpp \
    KDSoapServerCustomVerbRequestInterface.cpp

DEFINES += KDSOAP_BUILD_KDSOAPSERVER_LIB

# We use the soap client library, for xml parsing
INCLUDEPATH += . $${TOP_SOURCE_DIR}/src
DEPENDPATH += . $${TOP_SOURCE_DIR}/src
LIBS        += -L$$DESTDIR -l$$KDSOAPLIB

# installation targets:
target.path = $$INSTALL_PREFIX/lib$$LIB_SUFFIX
INSTALLS += target

# Mac frameworks
macx:lib_bundle: {
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$INSTALLHEADERS
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}
