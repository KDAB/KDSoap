# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(groupwise_wsdl)

set(WSDL_FILES groupwise.wsdl)
set(groupwise_wsdl_SRCS test_groupwise_wsdl.cpp wsdl_groupwise.cpp)

add_unittest(${groupwise_wsdl_SRCS})
