#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2016-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(ws_addressing_support)

set(WSDL_FILES wsaddressing.wsdl)
set(ws_addressing_support_SRCS test_wsaddressing.cpp)

add_unittest(${ws_addressing_support_SRCS})
