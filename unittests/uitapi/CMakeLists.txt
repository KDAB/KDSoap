#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2016-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(uitapi_SRCS test_uitapi.cpp)
set(WSDL_FILES uitapi.wsdl)
add_unittest(${uitapi_SRCS})
