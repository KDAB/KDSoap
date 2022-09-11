#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(ihc_wsdl_SRCS test_ihc.cpp)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server)
set(WSDL_FILES resourceinteraction.wsdl)
add_unittest(${ihc_wsdl_SRCS})
