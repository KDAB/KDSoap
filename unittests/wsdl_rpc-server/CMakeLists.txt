## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2017-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

set(WSDL_FILES rpcexample.wsdl sayhello.wsdl)
set(wsdl_rpc_SRCS test_wsdl_rpc_server.cpp)

set(EXTRA_LIBS kdsoap-server)
set(KSWSDL2CPP_OPTION "-server")

add_unittest(${wsdl_rpc_SRCS})
