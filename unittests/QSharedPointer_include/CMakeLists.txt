#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(QSharedPointer_include_SRCS test_qsharedpointer_include.cpp)

set(WSDL_FILES test_qsharedpointer_include_wsdl.wsdl)

add_unittest(${QSharedPointer_include_SRCS})
