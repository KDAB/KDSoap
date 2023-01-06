#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(unqualified_formdefault_SRCS test_unqualified.cpp)
set(WSDL_FILES unqualified.wsdl)
add_unittest(${unqualified_formdefault_SRCS})
