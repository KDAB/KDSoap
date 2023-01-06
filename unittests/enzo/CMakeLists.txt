#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(enzo_SRCS test_enzo.cpp)
set(WSDL_FILES EnzoService.wsdl)
add_unittest(${enzo_SRCS})
