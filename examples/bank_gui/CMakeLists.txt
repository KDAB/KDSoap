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

project(bank_gui)

set(bank_gui_SRCS bank_gui.cpp mainwindow.cpp)

KDSOAP_GENERATE_WSDL(bank_gui_SRCS BLZService.wsdl)

add_executable(bank_gui ${bank_gui_SRCS} resources.qrc)
target_link_libraries(bank_gui ${QT_LIBRARIES} kdsoap)
