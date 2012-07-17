project(helloworld_server)

set(helloworld_server_SRCS main.cpp)

add_executable(helloworld_server ${helloworld_server_SRCS})
target_link_libraries(helloworld_server ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY})
