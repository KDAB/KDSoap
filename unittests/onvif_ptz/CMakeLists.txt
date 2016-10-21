set(onvif_ptz_SRCS test_onvif_ptz.cpp)
set(WSDL_FILES ptz.wsdl)
set(KSWSDL2CPP_OPTION "-server")
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${onvif_ptz_SRCS})

