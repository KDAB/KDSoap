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
  KDSoapMessageAddressingProperties.cpp
  KDSoapEndpointReference.cpp
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
    DESTINATION ${INSTALL_INCLUDE_DIR}/KDSoapClient
  )

  install(TARGETS kdsoap EXPORT KDSoapTargets
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
    )

endif()
