## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2015-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

KDWSDL_OPTIONS = -server

g++-*: QMAKE_CXXFLAGS += -Wshadow

include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_prefix.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = kdtest.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

