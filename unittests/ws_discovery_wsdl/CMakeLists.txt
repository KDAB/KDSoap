project(ws_discovery_wsdl)

set(KSWSDL2CPP_OPTION -use-local-files-only)
set(WSDL_FILES ws_discovery200504.wsdl)
set(ws_discovery_wsdl_SRCS test_ws_discovery_wsdl.cpp wsdl_ws_discovery200504.cpp)

add_unittest(${ws_discovery_wsdl_SRCS})
