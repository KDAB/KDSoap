set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server)
set(WSDL_FILES kdtest.wsdl)
add_unittest(test_prefix.cpp)

