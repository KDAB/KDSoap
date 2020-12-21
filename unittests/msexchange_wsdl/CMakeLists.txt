project(msexchange_wsdl)

set(WSDL_FILES Services.wsdl)
set(msexchange_wsdl_SRCS msexchange_wsdl.cpp)

add_unittest(${msexchange_wsdl_SRCS})
