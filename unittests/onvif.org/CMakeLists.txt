
set(onvif.org_SRCS test_onvif.cpp)
set(WSDL_FILES devicemgmt.wsdl)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${onvif.org_SRCS})

