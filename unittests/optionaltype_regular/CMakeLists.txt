#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(optionaltype_regular_SRCS test_optionaltype_regular.cpp)
set(WSDL_FILES test.wsdl)
set(KSWSDL2CPP_OPTION -namespace KDAB)
add_unittest(${optionaltype_regular_SRCS})
