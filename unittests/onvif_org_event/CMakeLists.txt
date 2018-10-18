set(onvif_org_event_SRCS test_onvif_org_event.cpp)
set(WSDL_FILES event.wsdl)

get_filename_component(KDWSDL_IMPORT_PATH . ABSOLUTE)
set(KSWSDL2CPP_OPTION -server -namespace events_ver10 -import-path "${KDWSDL_IMPORT_PATH}/onvif.org" -use-local-files-only)
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)

add_unittest(${onvif_org_event_SRCS})
