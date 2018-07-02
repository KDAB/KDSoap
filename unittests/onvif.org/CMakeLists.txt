
set(onvif.org_SRCS test_onvif.cpp)
set(WSDL_FILES devicemgmt.wsdl)
get_filename_component(KDWSDL_IMPORT_PATH ./ ABSOLUTE)
set(KSWSDL2CPP_OPTION -server -import-path "${KDWSDL_IMPORT_PATH}" -use-local-files-only)
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${onvif.org_SRCS})

