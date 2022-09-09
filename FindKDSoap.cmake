#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2011-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

include(FindPackageHandleStandardArgs)

find_library(
    KDSoap_LIBRARIES
    NAMES KDSoap kdsoap
    PATH_SUFFIXES bin
)

find_path(
    KDSoap_INCLUDE_DIR
    NAMES KDSoapClient/KDSoapValue.h
    PATH_SUFFIXES include src
)

find_program(
    KDSoap_CODEGENERATOR
    NAMES kdwsdl2cpp
    PATH_SUFFIXES bin
)

mark_as_advanced(KDSoap_LIBRARIES KDSoap_INCLUDE_DIR KDSoap_CODEGENERATOR)

find_package_handle_standard_args(
    KDSoap
    DEFAULT_MSG
    KDSoap_LIBRARIES
    KDSoap_INCLUDE_DIR
    KDSoap_CODEGENERATOR
)
