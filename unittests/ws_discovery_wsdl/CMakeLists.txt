## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

project(ws_discovery_wsdl)

set(KSWSDL2CPP_OPTION -use-local-files-only -no-sync -no-async -no-async-jobs)
set(WSDL_FILES ws_discovery200504.wsdl)
set(ws_discovery_wsdl_SRCS test_ws_discovery_wsdl.cpp wsdl_ws_discovery200504.cpp)

add_unittest(${ws_discovery_wsdl_SRCS})
