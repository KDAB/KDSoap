# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(calc_SRCS test_calc.cpp)
set(WSDL_FILES calc.wsdl)
add_unittest(${calc_SRCS})
