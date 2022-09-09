#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(tech3356_wsdl_SRCS test_tech3356.cpp)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server)
set(WSDL_FILES transformMedia-V1_0_7.wsdl)
add_unittest(${tech3356_wsdl_SRCS})
