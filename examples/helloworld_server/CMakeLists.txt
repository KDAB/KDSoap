# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(helloworld_server)

set(KSWSDL2CPP_OPTION "-server")
set(helloworld_server_SRCS main.cpp helloworld_serverobject.cpp)

kdsoap_generate_wsdl(helloworld_server_SRCS helloworld.wsdl)

add_executable(
    helloworld_server
    ${helloworld_server_SRCS}
)
target_link_libraries(
    helloworld_server ${QT_LIBRARIES} kdsoap kdsoap-server
)
