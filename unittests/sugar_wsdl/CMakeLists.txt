project(sugar_wsdl)

set(WSDL_FILES sugarcrm.wsdl)
set(sugar_wsdl_SRCS test_sugar_wsdl.cpp)

add_unittest(${sugar_wsdl_SRCS})
