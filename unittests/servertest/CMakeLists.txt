project(servertest)

set(servertest_SRCS servertest.cpp)
set(EXTRA_LIBS kdsoap-server)
add_unittest(${servertest_SRCS})

