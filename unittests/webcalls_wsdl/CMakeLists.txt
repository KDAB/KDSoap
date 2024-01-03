# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(webcalls_wsdl)

set(WSDL_FILES BFGlobalService.wsdl BLZService.wsdl OrteLookup.wsdl)
set(webcalls_wsdl_SRCS test_webcalls_wsdl.cpp)

add_unittest(${webcalls_wsdl_SRCS})
