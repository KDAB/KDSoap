# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(element_ns_wsdl_SRCS test_element_ns_wsdl.cpp)
set(WSDL_FILES test.wsdl)
add_unittest(${element_ns_wsdl_SRCS})
