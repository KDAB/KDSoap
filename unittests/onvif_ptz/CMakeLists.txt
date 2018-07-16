set(onvif_ptz_SRCS test_onvif_ptz.cpp)
set(WSDL_FILES ptz.wsdl)
get_filename_component(KDWSDL_IMPORT_PATH ../onvif.org/ ABSOLUTE)
set(KSWSDL2CPP_OPTION -server -import-path "${KDWSDL_IMPORT_PATH}" -use-local-files-only)
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${onvif_ptz_SRCS})

