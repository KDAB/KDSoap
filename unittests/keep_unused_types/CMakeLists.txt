## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2017-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

project(keep_unused_types)

#this is the option given to KDWSDL2CPP when generating cpp from wsdl
set(KSWSDL2CPP_OPTION "-keep-unused-types")

set(WSDL_FILES keep_unused_types.wsdl)
set(keep_unused_types_SRCS keep_unused_types.cpp)

add_unittest(${keep_unused_types_SRCS})
