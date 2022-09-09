#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(messagereader)

set(messagereader_SRCS test_messagereader.cpp)
add_unittest(${messagereader_SRCS})
