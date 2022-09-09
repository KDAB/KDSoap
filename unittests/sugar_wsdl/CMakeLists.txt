#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(sugar_wsdl)

set(WSDL_FILES sugarcrm.wsdl)
set(sugar_wsdl_SRCS test_sugar_wsdl.cpp)

add_unittest(${sugar_wsdl_SRCS})
