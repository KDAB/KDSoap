#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(servertest)

set(servertest_SRCS test_serverlib.cpp)
set(EXTRA_LIBS kdsoap-server)
add_unittest(${servertest_SRCS})
