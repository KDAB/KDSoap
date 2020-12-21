set(encapsecurity_SRCS test_encapsecurity.cpp)
set(WSDL_FILES authstateless.wsdl)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server)
add_unittest(${encapsecurity_SRCS})

