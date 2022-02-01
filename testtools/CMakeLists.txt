## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

find_package(Qt${Qt_VERSION_MAJOR} ${QT_MIN_VERSION} COMPONENTS Xml CONFIG REQUIRED)
list(APPEND QT_LIBRARIES Qt${Qt_VERSION_MAJOR}::Xml)

include_directories(.. ../src/KDSoapClient)

set(testtools_srcs httpserver_p.cpp testtools.qrc)

add_library(testtools STATIC ${testtools_srcs})
target_link_libraries(testtools ${QT_LIBRARIES})
if(Qt5_POSITION_INDEPENDENT_CODE)
  set_target_properties(testtools PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
