#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

project(wsdl_document)
set(KSWSDL2CPP_OPTION -server)

set(WSDL_FILES thomas-bayer.wsdl mywsdl_document.wsdl)
set(wsdl_document_SRCS test_wsdl_document.cpp)
set(EXTRA_LIBS kdsoap-server)

add_unittest(${wsdl_document_SRCS})
