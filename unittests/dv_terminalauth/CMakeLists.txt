#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2017-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(dv_terminalauth_SRCS test_dv_terminalauth.cpp)
set(WSDL_FILES WS_DV_TerminalAuth.wsdl)
add_unittest(${dv_terminalauth_SRCS})
