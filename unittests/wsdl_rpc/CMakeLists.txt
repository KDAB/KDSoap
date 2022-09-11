#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(wsdl_rpc)

set(WSDL_FILES mywsdl_rpc.wsdl)
set(wsdl_rpc_SRCS test_wsdl_rpc.cpp)

add_unittest(${wsdl_rpc_SRCS})
