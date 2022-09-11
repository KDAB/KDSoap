#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(default_attribute_value_wsdl_SRCS test_default_attribute_value_wsdl.cpp)
set(WSDL_FILES default_attribute_value.wsdl)
set(KSWSDL2CPP_OPTION -optional-element-type boost-optional)
add_unittest(${default_attribute_value_wsdl_SRCS})
