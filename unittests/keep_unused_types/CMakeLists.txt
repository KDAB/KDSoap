#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2017-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(keep_unused_types)

#this is the option given to KDWSDL2CPP when generating cpp from wsdl
set(KSWSDL2CPP_OPTION "-keep-unused-types")

set(WSDL_FILES keep_unused_types.wsdl)
set(keep_unused_types_SRCS keep_unused_types.cpp)

add_unittest(${keep_unused_types_SRCS})
