project(builtinhttp)
set(builtinhttp_SRCS builtinhttp.cpp)
set(EXTRA_LIBS kdsoap-server ${QT_QTXML_LIBRARY})
add_unittest(${builtinhttp_SRCS} )
