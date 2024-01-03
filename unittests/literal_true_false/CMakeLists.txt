# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(literal_true_false_SRCS test_literal.cpp)
set(WSDL_FILES literal.wsdl)
add_unittest(${literal_true_false_SRCS})
