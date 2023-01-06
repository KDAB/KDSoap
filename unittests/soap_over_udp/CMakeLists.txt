#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2019-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(soap_over_udp)

set(KSWSDL2CPP_OPTION -use-local-files-only)
set(WSDL_FILES wsdd-discovery-200901.wsdl)
set(soap_over_udp_SRCS test_soap_over_udp.cpp wsdl_wsdd-discovery-200901.cpp)

add_unittest(${soap_over_udp_SRCS})
