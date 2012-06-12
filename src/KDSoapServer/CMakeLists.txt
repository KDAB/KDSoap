set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})

add_definitions(-DKDSOAPSERVER_STATICLIB -DKDSOAP_STATICLIB -DKDSOAP_BUILD_KDSOAPSERVER_LIB)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(.. ../KDSoapClient)

set(SOURCES
	KDSoapDelayedResponseHandle.cpp
	KDSoapServer.cpp
	KDSoapServerObjectInterface.cpp
	KDSoapServerSocket.cpp
	KDSoapServerThread.cpp
	KDSoapServerAuthInterface.cpp
	KDSoapSocketList.cpp
	KDSoapThreadPool.cpp
)

set(HEADERS
	KDSoapServer.h
	KDSoapServerSocket_p.h
	KDSoapThreadPool.h
)

qt4_generate_moc_cpp(${HEADERS})

qt4_wrap_cpp(MOC_SOURCES KDSoapServerThread_p.h KDSoapSocketList_p.h)

add_library(kdsoap-server ${${PROJECT_NAME}_LIBRARY_MODE} ${MOC_SOURCES} ${SOURCES})
target_link_libraries(kdsoap-server kdsoap ${QT_LIBRARIES})
set_target_properties(kdsoap-server PROPERTIES VERSION ${${PROJECT_NAME}_VERSION})

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