# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(webcalls)

set(webcalls_SRCS test_webcalls.cpp)
add_unittest(${webcalls_SRCS})
