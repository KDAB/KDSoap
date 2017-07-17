set(SRCS test_onvif_media.cpp)
set(WSDL_FILES media.wsdl)
set(KSWSDL2CPP_OPTION -server -namespace media)
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${SRCS})

