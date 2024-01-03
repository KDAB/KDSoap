# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(msexchange_wsdl)

set(WSDL_FILES Services.wsdl)
set(msexchange_wsdl_SRCS test_msexchange_wsdl.cpp)

add_unittest(${msexchange_wsdl_SRCS})
