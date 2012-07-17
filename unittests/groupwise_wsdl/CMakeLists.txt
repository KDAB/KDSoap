project(groupwise_wsdl)

set(WSDL_FILES groupwise.wsdl)
set(groupwise_wsdl_SRCS  test_groupwise_wsdl.cpp wsdl_groupwise.cpp )
set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${groupwise_wsdl_SRCS} )
