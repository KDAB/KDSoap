#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(logbook_wsdl)

set(WSDL_FILES logbookifv3.wsdl)
set(logbook_wsdl_SRCS test_logbook_wsdl.cpp wsdl_logbookifv3.cpp)

add_unittest(${logbook_wsdl_SRCS})
