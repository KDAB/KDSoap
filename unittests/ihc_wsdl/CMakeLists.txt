
set(ihc_wsdl_SRCS test_ihc.cpp)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server)
set(WSDL_FILES resourceinteraction.wsdl)
add_unittest(${ihc_wsdl_SRCS})

