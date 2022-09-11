#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2018-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(empty_list_wsdl_SRCS test_empty_list_wsdl.cpp)
set(WSDL_FILES empty_list.wsdl)
add_unittest(${empty_list_wsdl_SRCS})
