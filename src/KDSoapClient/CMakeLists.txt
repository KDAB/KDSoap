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
)

add_library(kdsoap STATIC ${HEADERS} ${SOURCES})
target_link_libraries(kdsoap ${QT_LIBRARIES})
set_target_properties(kdsoap PROPERTIES VERSION ${${PROJECT_NAME}_VERSION})

if(KDSoap_IS_ROOT_PROJECT)
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
endif()
