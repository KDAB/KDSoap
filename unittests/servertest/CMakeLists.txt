project(servertest)

set(servertest_SRCS servertest.cpp)
set(EXTRA_LIBS kdsoap-server ${QT_QTXML_LIBRARY} )
add_unittest(${servertest_SRCS} )

