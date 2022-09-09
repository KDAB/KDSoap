project(basic)
#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(basic_SRCS test_basic.cpp)
add_unittest(${basic_SRCS})
