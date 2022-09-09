#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(encapsecurity_SRCS test_encapsecurity.cpp)
set(WSDL_FILES authstateless.wsdl)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server)
add_unittest(${encapsecurity_SRCS})
