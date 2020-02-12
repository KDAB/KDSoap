project(webcalls_wsdl)

set(WSDL_FILES BFGlobalService.wsdl  BLZService.wsdl  OrteLookup.wsdl  soapclient.com_emptySA.wsdl)
set(webcalls_wsdl_SRCS webcalls_wsdl.cpp)

set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${webcalls_wsdl_SRCS})
