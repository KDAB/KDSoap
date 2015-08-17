set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})

if(KDSoap_STATIC)
  add_definitions(-DKDSOAP_STATICLIB)
else()
  add_definitions(-DKDSOAP_BUILD_KDSOAP_LIB)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCES
  KDSoapMessage.cpp
  KDSoapClientInterface.cpp
  KDSoapPendingCall.cpp
  KDSoapPendingCallWatcher.cpp
  KDSoapClientThread.cpp
  KDSoapValue.cpp
  KDSoapAuthentication.cpp
  KDSoapNamespaceManager.cpp
  KDSoapMessageWriter.cpp
  KDSoapMessageReader.cpp
  KDDateTime.cpp
  KDSoapNamespacePrefixes.cpp
  KDSoapJob.cpp
  KDSoapSslHandler.cpp
  KDSoapReplySslHandler.cpp
  KDSoapFaultException.cpp
)

add_library(kdsoap ${KDSoap_LIBRARY_MODE} ${SOURCES})
target_link_libraries(kdsoap ${QT_LIBRARIES})
set_target_properties(kdsoap PROPERTIES VERSION ${${PROJECT_NAME}_VERSION})

if(KDSoap_IS_ROOT_PROJECT)
  include(ECMGenerateHeaders)
  ecm_generate_headers(client_HEADERS
    ORIGINAL
      CAMELCASE
    HEADER_NAMES
      KDSoapMessage
      KDSoapGlobal
      KDSoap
      KDDateTime
      KDSoapJob
      KDSoapClientInterface
      KDSoapNamespaceManager
      KDSoapSslHandler
      KDSoapValue
      KDSoapPendingCallWatcher
      KDSoapFaultException
      KDSoapPendingCall
      KDSoapAuthentication
    REQUIRED_HEADERS
      required_HEADERS
  )

  #handle forwarding headers for the case where more than 1 class lives in a .h
  #configure_file is the simple way to file copy in cmake
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/KDSoapMessage ${CMAKE_CURRENT_BINARY_DIR}/KDSoapHeaders COPYONLY)
  list(APPEND client_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/KDSoapHeaders)
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/KDSoapValue ${CMAKE_CURRENT_BINARY_DIR}/KDSoapValueList COPYONLY)
  list(APPEND client_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/KDSoapValueList)

  #combine required headers into 1 big convenience header
  set(COMBINED_H ${CMAKE_CURRENT_BINARY_DIR}/KDSoapClient)
  foreach(_header ${required_HEADERS})
    get_filename_component(_base ${_header} NAME)
    file(APPEND ${COMBINED_H} "#include \"${_base}\"\n")
  endforeach()

  install(FILES
    ${client_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/KDSoapClient
    KDSoapMessage.h
    KDSoapClientInterface.h
    KDSoapPendingCall.h
    KDSoapPendingCallWatcher.h
    KDSoapValue.h
    KDSoapGlobal.h
    KDSoapJob.h
    KDSoapAuthentication.h
    KDSoapNamespaceManager.h
    KDDateTime.h
    KDSoap.h
    KDSoapSslHandler.h
    KDSoapFaultException.h
    DESTINATION ${INSTALL_INCLUDE_DIR}/KDSoapClient
  )

  install(TARGETS kdsoap EXPORT KDSoapTargets
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
    )

endif()
