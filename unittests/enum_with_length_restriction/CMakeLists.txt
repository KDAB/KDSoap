#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(enum_with_length_restriction_SRCS test_enum_length_restriction.cpp)
set(WSDL_FILES test_enum.wsdl)
add_unittest(${enum_with_length_restriction_SRCS})
