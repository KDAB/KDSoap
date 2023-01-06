#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(multiple_input_param_SRCS test_multiple_input_param.cpp)
set(WSDL_FILES helloworldextended.wsdl)
add_unittest(${multiple_input_param_SRCS})
