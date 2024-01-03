# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(dwservice_wsdl)
set(KSWSDL2CPP_OPTION -namespace KDAB)

set(WSDL_FILES DWService.wsdl)
set(dwservice_wsdl_SRCS test_dwservice_wsdl.cpp)

add_unittest(${dwservice_wsdl_SRCS})
