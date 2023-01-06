#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2019-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(bank_gui)

set(bank_gui_SRCS bank_gui.cpp mainwindow.cpp)

kdsoap_generate_wsdl(bank_gui_SRCS BLZService.wsdl)

add_executable(
    bank_gui
    ${bank_gui_SRCS} resources.qrc
)
target_link_libraries(
    bank_gui ${QT_LIBRARIES} kdsoap
)
