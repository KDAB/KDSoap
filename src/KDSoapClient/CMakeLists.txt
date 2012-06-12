set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})

add_definitions(-DKDSOAPSERVER_STATICLIB -DKDSOAP_STATICLIB -DKDSOAP_BUILD_KDSOAP_LIB)

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
)

set(HEADERS
	KDSoapClientThread_p.h
	KDSoapJob.h
)

qt4_wrap_cpp(MOC_SOURCES ${HEADERS})

qt4_generate_moc_cpp(KDSoapPendingCallWatcher.h KDSoapClientInterface_p.h KDSoapJob.h)

add_library(kdsoap STATIC ${MOC_SOURCES} ${SOURCES})
target_link_libraries(kdsoap ${QT_LIBRARIES})
set_target_properties(kdsoap PROPERTIES VERSION ${${PROJECT_NAME}_VERSION})

install(FILES
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
	KDSoap
	DESTINATION include
)

install(TARGETS kdsoap
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)