project(helloworld_server)

set(KSWSDL2CPP_OPTION "-server")
set(helloworld_server_SRCS main.cpp helloworld_server.h)

KDSOAP_GENERATE_WSDL(helloworld_server_SRCS helloworld.wsdl)

add_executable(helloworld_server ${helloworld_server_SRCS})
target_link_libraries(helloworld_server ${QT_LIBRARIES} kdsoap kdsoap-server)
