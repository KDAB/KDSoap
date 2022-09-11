#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2017-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(date_example_SRCS test_date_example.cpp)
set(WSDL_FILES date_example.wsdl)
add_unittest(${date_example_SRCS})
