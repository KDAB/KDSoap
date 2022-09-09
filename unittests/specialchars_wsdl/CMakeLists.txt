#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(specialchars_wsdl_SRCS test_specialchars_wsdl.cpp)
set(WSDL_FILES test.wsdl)
add_unittest(${specialchars_wsdl_SRCS})
