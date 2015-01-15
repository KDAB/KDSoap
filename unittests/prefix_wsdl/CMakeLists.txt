set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server ${QT_QTXML_LIBRARY} )
set(WSDL_FILES kdtest.wsdl)
add_unittest(test_prefix.cpp)

