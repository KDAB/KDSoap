
set(soap12_SRCS test_soap12.cpp)
set(WSDL_FILES soap12.wsdl)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${soap12_SRCS})

