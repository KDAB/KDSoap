
set(clearbooks_SRCS test_clearbooks.cpp)
set(WSDL_FILES clearbooks.wsdl)
set(KSWSDL2CPP_OPTION -server)
set(EXTRA_LIBS kdsoap-server)
add_unittest(${clearbooks_SRCS})

