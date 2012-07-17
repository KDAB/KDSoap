project(helloworld_server)

set(helloworld_server_SRCS main.cpp)
set(helloworld_server_HDRS helloworld_server.h)

KDSOAP_GENERATE_SERVER_WSDL(helloworld_server_SRCS helloworld.wsdl)

qt4_wrap_cpp( helloworld_server_MOCS ${helloworld_server_HDRS} )

add_executable(helloworld_server ${helloworld_server_SRCS} ${helloworld_server_MOCS} )
target_link_libraries(helloworld_server ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} kdsoap kdsoap-server)
