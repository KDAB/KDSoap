project(ws_addressing_support)

set(WSDL_FILES wsaddressing.wsdl)
set(ws_addressing_support_SRCS wsaddressingtest.cpp)

add_unittest(${ws_addressing_support_SRCS})
