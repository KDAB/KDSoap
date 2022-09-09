#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2015-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server)
set(WSDL_FILES kdtest.wsdl)
add_unittest(test_prefix.cpp)
