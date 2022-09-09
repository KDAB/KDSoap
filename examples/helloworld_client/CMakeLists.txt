#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(helloworld_client)

set(helloworld_client_SRCS main.cpp helloworld_client.h)

kdsoap_generate_wsdl(helloworld_client_SRCS ../helloworld_server/helloworld.wsdl)

add_executable(
    helloworld_client
    ${helloworld_client_SRCS}
)
target_link_libraries(
    helloworld_client ${QT_LIBRARIES} kdsoap
)
