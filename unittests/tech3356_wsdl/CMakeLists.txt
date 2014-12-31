
set(tech3356_wsdl_SRCS test_tech3356.cpp)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS kdsoap-server ${QT_QTXML_LIBRARY} )
set(WSDL_FILES transformMedia-V1_0_7.wsdl)
add_unittest(${tech3356_wsdl_SRCS})

