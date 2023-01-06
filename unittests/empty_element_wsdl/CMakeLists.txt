#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(empty_element_wsdl_SRCS test_empty_element_wsdl.cpp)
set(WSDL_FILES empty_element.wsdl)
add_unittest(${empty_element_wsdl_SRCS})
