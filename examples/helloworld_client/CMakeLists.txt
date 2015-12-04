project(helloworld_client)

include_directories(${QT_QTGUI_INCLUDE_DIR})

set(helloworld_client_SRCS main.cpp)

set(helloworld_client_HRDS helloworld_client.h )
qt4_wrap_cpp( helloworld_client_MOCS ${helloworld_client_HRDS} )

KDSOAP_GENERATE_WSDL(helloworld_client_SRCS helloworld.wsdl )

add_executable(helloworld_client ${helloworld_client_SRCS} ${helloworld_client_MOCS} )
target_link_libraries(helloworld_client ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTWIDGETS_LIBRARY} kdsoap)
