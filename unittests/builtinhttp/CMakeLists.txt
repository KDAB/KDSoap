#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(builtinhttp)
set(builtinhttp_SRCS test_builtinhttp.cpp)
set(EXTRA_LIBS kdsoap-server)
add_unittest(${builtinhttp_SRCS})
