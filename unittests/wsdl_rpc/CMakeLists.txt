project(wsdl_rpc)

set(WSDL_FILES mywsdl_rpc.wsdl)
set(wsdl_rpc_SRCS test_wsdl_rpc.cpp)

set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${wsdl_rpc_SRCS} )
