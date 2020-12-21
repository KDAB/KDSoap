## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2019-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

project(soap_over_udp)

set(KSWSDL2CPP_OPTION -use-local-files-only)
set(WSDL_FILES wsdd-discovery-200901.wsdl)
set(soap_over_udp_SRCS test_soap_over_udp.cpp wsdl_wsdd-discovery-200901.cpp)

add_unittest(${soap_over_udp_SRCS})
