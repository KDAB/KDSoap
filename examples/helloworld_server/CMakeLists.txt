## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

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
