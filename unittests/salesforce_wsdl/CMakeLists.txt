project(salesforce_wsdl)

set(WSDL_FILES salesforce-partner.wsdl)
set(salesforce_wsdl_SRCS salesforce_wsdl.cpp)

add_unittest(${salesforce_wsdl_SRCS})
