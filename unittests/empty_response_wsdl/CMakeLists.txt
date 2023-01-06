#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(empty_response_wsdl_SRCS test_issue1.cpp)
set(WSDL_FILES test.wsdl)
add_unittest(${empty_response_wsdl_SRCS})
