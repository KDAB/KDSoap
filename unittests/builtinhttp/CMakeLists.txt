project(builtinhttp)
set(builtinhttp_SRCS builtinhttp.cpp)
set(EXTRA_LIBS kdsoap-server)
add_unittest(${builtinhttp_SRCS})
