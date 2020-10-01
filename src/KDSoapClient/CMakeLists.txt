set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})
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
  KDSoapMessageAddressingProperties.cpp
  KDSoapEndpointReference.cpp
  KDQName.cpp
  KDSoapUdpClient.cpp
)

add_library(kdsoap ${KDSoap_LIBRARY_MODE} ${SOURCES})
if(${PROJECT_NAME}_STATIC)
  target_compile_definitions(kdsoap PUBLIC KDSOAP_STATICLIB)
else()
  target_compile_definitions(kdsoap PRIVATE KDSOAP_BUILD_KDSOAP_LIB)
endif()
target_link_libraries(kdsoap ${QT_LIBRARIES})
target_include_directories(kdsoap INTERFACE "$<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>")
set_target_properties(kdsoap PROPERTIES
  SOVERSION ${${PROJECT_NAME}_SOVERSION}
  VERSION ${${PROJECT_NAME}_VERSION}
)

#version libraries on Windows
if(WIN32)
  set(postfix ${${PROJECT_NAME}_SOVERSION})
  set(CMAKE_RELEASE_POSTFIX ${postfix})
  set_target_properties(kdsoap PROPERTIES RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})
  #append 'd' to debug libraries
  string(CONCAT postfix ${postfix} "d")
  set(CMAKE_DEBUG_POSTFIX ${postfix})
  set_target_properties(kdsoap PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endif()

if(KDSoap_IS_ROOT_PROJECT)
  include(ECMGenerateHeaders)
  ecm_generate_headers(client_HEADERS
    ORIGINAL
      CAMELCASE
    HEADER_NAMES
      KDSoapGlobal
      KDSoapMessage,KDSoapHeaders
      KDSoap
      KDDateTime
      KDSoapJob
      KDSoapClientInterface
      KDSoapNamespaceManager
      KDSoapSslHandler
      KDSoapValue,KDSoapValueList
      KDSoapPendingCallWatcher
      KDSoapFaultException
      KDSoapMessageAddressingProperties
      KDSoapEndpointReference
      KDSoapPendingCall
      KDSoapAuthentication
      KDQName
      KDSoapUdpClient
    COMMON_HEADER
      KDSoapClient
  )

  install(FILES
    ${client_HEADERS}
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
    KDSoapMessageAddressingProperties.h
    KDSoapEndpointReference.h
    KDQName.h
    KDSoapUdpClient.h
    DESTINATION ${INSTALL_INCLUDE_DIR}/KDSoapClient
  )

  install(TARGETS kdsoap EXPORT KDSoapTargets
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )

  if(MSVC AND NOT ${PROJECT_NAME}_STATIC)
    install(FILES "$<TARGET_PDB_FILE_DIR:kdsoap>/$<TARGET_PDB_FILE_NAME:kdsoap>" DESTINATION ${INSTALL_LIBRARY_DIR} CONFIGURATIONS Debug RelWithDebInfo)
  endif()

endif()
