#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2015-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(clearbooks_SRCS test_clearbooks.cpp)
set(WSDL_FILES clearbooks.wsdl)
set(KSWSDL2CPP_OPTION -server)
set(EXTRA_LIBS kdsoap-server)
add_unittest(${clearbooks_SRCS})
