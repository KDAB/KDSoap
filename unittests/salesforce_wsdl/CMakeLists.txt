project(salesforce_wsdl)

set(WSDL_FILES salesforce-partner.wsdl)
set(salesforce_wsdl_SRCS salesforce_wsdl.cpp )

set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${salesforce_wsdl_SRCS} )
