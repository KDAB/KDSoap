#
# This file is part of the KD Soap project..
#
# SPDX-FileCopyrightText: 2012-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

include_directories(.. ../KDSoapClient)

set(SOURCES
    KDSoapDelayedResponseHandle.cpp
    KDSoapServer.cpp
    KDSoapServerObjectInterface.cpp
    KDSoapServerSocket.cpp
    KDSoapServerThread.cpp
    KDSoapServerThread.cpp
    KDSoapServerThread.cpp
    KDSoapServerAuthInterface.cpp
    KDSoapServerRawXMLInterface.cpp
    KDSoapServerCustomVerbRequestInterface.cpp
    KDSoapSocketList.cpp
    KDSoapThreadPool.cpp
)

set_source_files_properties(KDSoapServerObjectInterface.cpp PROPERTIES SKIP_AUTOMOC TRUE)
add_library(
    kdsoap-server
    ${KDSoap_LIBRARY_MODE} ${SOURCES}
)
set_target_properties(kdsoap-server PROPERTIES OUTPUT_NAME "kdsoap-server${${PROJECT_NAME}_LIBRARY_QTID}")
if(${PROJECT_NAME}_STATIC)
    target_compile_definitions(kdsoap-server PUBLIC KDSOAPSERVER_STATICLIB)
else()
    target_compile_definitions(kdsoap-server PRIVATE KDSOAP_BUILD_KDSOAPSERVER_LIB)
endif()
target_link_libraries(
    kdsoap-server kdsoap ${QT_LIBRARIES}
)
target_include_directories(
    kdsoap-server
    INTERFACE "$<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>"
)
set_target_properties(
    kdsoap-server PROPERTIES SOVERSION ${${PROJECT_NAME}_SOVERSION} VERSION ${${PROJECT_NAME}_VERSION}
)

#version libraries on Windows
if(WIN32)
    set(postfix ${${PROJECT_NAME}_SOVERSION})
    string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_BUILD_TYPE)
    if(${UPPER_BUILD_TYPE} MATCHES "^DEBUG")
        string(CONCAT postfix ${postfix} "d")
        set_target_properties(kdsoap-server PROPERTIES DEBUG_POSTFIX ${postfix})
    else()
        set_target_properties(kdsoap-server PROPERTIES ${UPPER_BUILD_TYPE}_POSTFIX ${postfix})
    endif()
endif()

if(KDSoap_IS_ROOT_PROJECT)
    include(ECMGenerateHeaders)
    ecm_generate_headers(
        server_HEADERS
        ORIGINAL
        CAMELCASE
        HEADER_NAMES
        KDSoapDelayedResponseHandle
        KDSoapServerGlobal
        KDSoapThreadPool
        KDSoapServerObjectInterface
        KDSoapServerAuthInterface
        KDSoapServerRawXMLInterface
        KDSoapServerCustomVerbRequestInterface
        COMMON_HEADER
        KDSoapServer
    )

    set(COMBINED_H ${CMAKE_CURRENT_BINARY_DIR}/KDSoapServer)
    file(APPEND ${COMBINED_H} "#include \"KDSoapServer.h\"\n")

    install(
        FILES ${server_HEADERS}
              KDSoapServer.h
              KDSoapServerAuthInterface.h
              KDSoapServerRawXMLInterface.h
              KDSoapServerCustomVerbRequestInterface.h
              KDSoapDelayedResponseHandle.h
              KDSoapServerObjectInterface.h
              KDSoapServerGlobal.h
              KDSoapThreadPool.h
        DESTINATION ${INSTALL_INCLUDE_DIR}/KDSoapServer
    )

    install(
        TARGETS kdsoap-server
        EXPORT KDSoapTargets
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
    )

    if(MSVC AND NOT ${PROJECT_NAME}_STATIC)
        install(
            FILES "$<TARGET_PDB_FILE_DIR:kdsoap-server>/$<TARGET_PDB_FILE_NAME:kdsoap-server>"
            DESTINATION ${INSTALL_LIBRARY_DIR}
            CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

endif()
