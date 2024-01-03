# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(enum_escape_SRCS test_enum.cpp)
set(WSDL_FILES test_enum.wsdl)
add_unittest(${enum_escape_SRCS})
