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
	KDSoapSocketList.cpp
	KDSoapThreadPool.cpp
)

set_source_files_properties(KDSoapServerObjectInterface.cpp PROPERTIES SKIP_AUTOMOC TRUE)
add_library(kdsoap-server ${KDSoap_LIBRARY_MODE} ${SOURCES})
target_link_libraries(kdsoap-server kdsoap ${QT_LIBRARIES})
set_target_properties(kdsoap-server PROPERTIES VERSION ${${PROJECT_NAME}_VERSION})

if(KDSoap_IS_ROOT_PROJECT)
	install(FILES
		KDSoapServer.h
		KDSoapServerAuthInterface.h
		KDSoapServerObjectInterface.h
		KDSoapServerGlobal.h
		DESTINATION include
	)

	install(TARGETS kdsoap-server
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
endif()
