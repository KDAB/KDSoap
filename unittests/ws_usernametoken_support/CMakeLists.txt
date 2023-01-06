#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2019-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(ws_usernametoken_support)

set(WSDL_FILES wsusernametoken.wsdl)
set(ws_usernametoken_support_SRCS test_wsusernametoken.cpp)

add_unittest(${ws_usernametoken_support_SRCS})
