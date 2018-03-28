set(enzo_SRCS test_enzo.cpp)
set(WSDL_FILES EnzoService.wsdl)
add_unittest(${enzo_SRCS})
