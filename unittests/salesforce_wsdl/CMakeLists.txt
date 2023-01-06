#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(salesforce_wsdl)

set(WSDL_FILES salesforce-partner.wsdl)
set(salesforce_wsdl_SRCS test_salesforce_wsdl.cpp)

add_unittest(${salesforce_wsdl_SRCS})
