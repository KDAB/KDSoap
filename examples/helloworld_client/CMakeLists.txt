project(helloworld_client)

set(helloworld_client_SRCS main.cpp)

KDSOAP_GENERATE_WSDL(helloworld_client_SRCS helloworld.wsdl )

add_executable(helloworld_client ${helloworld_client_SRCS})
target_link_libraries(helloworld_client ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY})
