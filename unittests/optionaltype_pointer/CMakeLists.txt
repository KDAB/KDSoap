#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(optionaltype_pointer_SRCS test_optionaltype_pointer.cpp)
set(KSWSDL2CPP_OPTION -optional-element-type raw-pointer)
set(WSDL_FILES test.wsdl)
add_unittest(${optionaltype_pointer_SRCS})
