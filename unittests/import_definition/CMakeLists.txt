#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(import_definition_SRCS test_import_definition.cpp)
set(WSDL_FILES import_definition.wsdl import_definition_wsdl.wsdl)
add_unittest(${import_definition_SRCS})
