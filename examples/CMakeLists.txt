## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2012-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} COMPONENTS Widgets CONFIG REQUIRED)
list(APPEND QT_LIBRARIES Qt${QT_MAJOR_VERSION}::Widgets)

include(${CMAKE_BINARY_DIR}/KDSoap/KDSoapMacros.cmake)
include_directories(../src/ ../src/KDSoapClient/ ../src/KDSoapServer/)

add_subdirectory(helloworld_client)
add_subdirectory(helloworld_server)
add_subdirectory(bank_gui)
add_subdirectory(bank_wsdl)
