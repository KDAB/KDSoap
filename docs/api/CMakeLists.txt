## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

find_file(QDOC_QTCORE_TAG qtcore.tags
  HINTS ${QT_INSTALL_DOCS}/qtcore
  HINTS ${QT_INSTALL_DATA}/doc/qtcore
)
if(QDOC_QTCORE_TAG)
  get_filename_component(QDOC_TAG_DIR ${QDOC_QTCORE_TAG} DIRECTORY)
  get_filename_component(QDOC_TAG_DIR ${QDOC_TAG_DIR} DIRECTORY)
endif()

file(GLOB _dox_deps *.dox *.html)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

#apidox generation using doxygen
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
)

add_custom_command(
  OUTPUT ${DOXYGEN_OUTPUT_DIR}/qch/kdsoap-api.qch
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  DEPENDS ${_dox_deps} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(kdsoap-api.qch ALL DEPENDS ${DOXYGEN_OUTPUT_DIR}/qch/kdsoap-api.qch)
add_custom_target(docs DEPENDS kdsoap-api.qch)

install(FILES ${DOXYGEN_OUTPUT_DIR}/qch/kdsoap-api.qch DESTINATION ${INSTALL_DOC_DIR})
