#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(bank_wsdl)

set(bank_wsdl_SRCS bank_wsdl.cpp)

kdsoap_generate_wsdl(bank_wsdl_SRCS BLZService.wsdl)

add_executable(
    bank_wsdl
    ${bank_wsdl_SRCS}
)
target_link_libraries(
    bank_wsdl ${QT_LIBRARIES} kdsoap
)
