project(helloworld_server)

set(helloworld_server_SRCS main.cpp)

KDSOAP_GENERATE_SERVER_WSDL(helloworld_server_SRCS helloworld.wsdl)

add_executable(helloworld_server ${helloworld_server_SRCS})
target_link_libraries(helloworld_server ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} kdsoap kdsoap-server)
