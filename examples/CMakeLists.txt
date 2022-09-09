#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

find_package(
    Qt${Qt_VERSION_MAJOR} ${QT_MIN_VERSION}
    COMPONENTS Widgets CONFIG
    REQUIRED
)
list(APPEND QT_LIBRARIES Qt${Qt_VERSION_MAJOR}::Widgets)

include(${CMAKE_BINARY_DIR}/KDSoap/KDSoapMacros.cmake)
include_directories(../src/ ../src/KDSoapClient/ ../src/KDSoapServer/)

add_subdirectory(helloworld_client)
add_subdirectory(helloworld_server)
add_subdirectory(bank_gui)
add_subdirectory(bank_wsdl)
