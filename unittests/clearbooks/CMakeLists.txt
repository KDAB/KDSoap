
set(clearbooks_SRCS test_clearbooks.cpp)
set(WSDL_FILES clearbooks.wsdl)
set(KSWSDL2CPP_OPTION -server)
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${clearbooks_SRCS})

