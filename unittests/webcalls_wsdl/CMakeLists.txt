project(webcalls_wsdl)

set(WSDL_FILES BFGlobalService.wsdl BLZService.wsdl OrteLookup.wsdl)
set(webcalls_wsdl_SRCS webcalls_wsdl.cpp)

add_unittest(${webcalls_wsdl_SRCS})
