set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})

if(KDSoap_STATIC)
  add_definitions(-DKDSOAPSERVER_STATICLIB)
else()
  add_definitions(-DKDSOAP_BUILD_KDSOAPSERVER_LIB)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
add_library(kdsoap-server ${KDSoap_LIBRARY_MODE} ${SOURCES})
target_link_libraries(kdsoap-server kdsoap ${QT_LIBRARIES})
set_target_properties(kdsoap-server PROPERTIES VERSION ${${PROJECT_NAME}_VERSION})

if(KDSoap_IS_ROOT_PROJECT)
  include(ECMGenerateHeaders)
  ecm_generate_headers(server_HEADERS
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

  install(FILES
    ${server_HEADERS}
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

  install(TARGETS kdsoap-server EXPORT KDSoapTargets
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )
endif()
