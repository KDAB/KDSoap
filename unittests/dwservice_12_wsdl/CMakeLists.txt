#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(dwservice_12_wsdl)

set(KSWSDL2CPP_OPTION -namespace KDAB)

set(WSDL_FILES DWService_12.wsdl)
set(dwservice_12_wsdl_SRCS test_dwservice_12_wsdl.cpp)

add_unittest(${dwservice_12_wsdl_SRCS})
