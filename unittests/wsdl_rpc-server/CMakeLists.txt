set(WSDL_FILES rpcexample.wsdl sayhello.wsdl)
set(wsdl_rpc_SRCS test_wsdl_rpc_server.cpp)

set(EXTRA_LIBS kdsoap-server)
set(KSWSDL2CPP_OPTION "-server")

add_unittest(${wsdl_rpc_SRCS})
