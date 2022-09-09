#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2021-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

# dot should come with Doxygen find_package(Doxygen)
if(DOXYGEN_DOT_EXECUTABLE)
    set(HAVE_DOT "YES")
else()
    set(HAVE_DOT "NO")
    message(
        STATUS
            "Unable to provide inheritance diagrams for the API documentation. To fix, install the graphviz project from https://www.graphviz.org"
    )
endif()

# latex
set(HAVE_LATEX "NO")
find_package(LATEX)
set_package_properties(
    LATEX PROPERTIES
    TYPE OPTIONAL
    DESCRIPTION "A document preparation system"
    URL "https://www.latex-project.org"
    PURPOSE "Provides for building a PS or PDF version of the API documentation"
)
if(LATEX_FOUND)
    if(MAKEINDEX_COMPILER)
        set(HAVE_LATEX "YES")
    else()
        message(
            STATUS
                "The LaTeX makeindex compiler could not be located. Unable to generate the buildsystem for LaTex documentation generation."
        )
    endif()
endif()

# qhelpgenerator
find_program(QHELPGEN_EXECUTABLE qhelpgenerator HINTS ${QT_INSTALL_BINS})
if(QHELPGEN_EXECUTABLE)
    set(HAVE_QHELPGEN "YES")
else()
    set(HAVE_QHELPGEN "NO")
    message(
        STATUS
            "Unable to generate the API documentation in qch format. To fix, install the qthelpgenerator program which comes with Qt."
    )
endif()

find_file(
    QDOC_QTCORE_TAG qtcore.tags
    HINTS ${QT_INSTALL_DOCS}/qtcore
    HINTS ${QT_INSTALL_DATA}/doc/qtcore
)
set(QDOC_TAG_DIR "<QDOC_TAG_DIR_not_found>")
if(QDOC_QTCORE_TAG)
    get_filename_component(QDOC_TAG_DIR ${QDOC_QTCORE_TAG} DIRECTORY)
    get_filename_component(QDOC_TAG_DIR ${QDOC_TAG_DIR} DIRECTORY)
endif()

file(GLOB _dox_deps *.dox *.html)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

#apidox generation using doxygen
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT_DIR}/qch/kdsoap-api.qch
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    DEPENDS ${_dox_deps} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(kdsoap-api.qch ALL DEPENDS ${DOXYGEN_OUTPUT_DIR}/qch/kdsoap-api.qch)
add_custom_target(docs DEPENDS kdsoap-api.qch)

set(QCH_INSTALL_DIR
    ${INSTALL_DOC_DIR}
    CACHE STRING "Install location of Qt Assistant help files."
)
install(
    FILES ${DOXYGEN_OUTPUT_DIR}/qch/kdsoap-api.qch
    DESTINATION ${QCH_INSTALL_DIR}
)
install(
    FILES ${DOXYGEN_OUTPUT_DIR}/kdsoap.tags
    DESTINATION ${INSTALL_DOC_DIR}
)
