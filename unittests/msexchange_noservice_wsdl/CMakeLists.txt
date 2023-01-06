#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(msexchange_noservice_wsdl_SRCS test_msexchange_noservice_wsdl.cpp)
set(WSDL_FILES Services_noservice.wsdl)
add_unittest(${msexchange_noservice_wsdl_SRCS})
