project(wsdl_rpc)

set(WSDL_FILES mywsdl_rpc.wsdl)
set(wsdl_rpc_SRCS test_wsdl_rpc.cpp)

add_unittest(${wsdl_rpc_SRCS})
