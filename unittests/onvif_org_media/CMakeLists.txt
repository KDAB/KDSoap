set(SRCS test_onvif_media.cpp)
set(WSDL_FILES media.wsdl)
get_filename_component(KDWSDL_IMPORT_PATH ../onvif.org/ ABSOLUTE)
set(KSWSDL2CPP_OPTION -server -namespace media  -import-path "${KDWSDL_IMPORT_PATH}" -use-local-files-only)
set(EXTRA_LIBS ${QT_QTXML_LIBRARY} kdsoap-server)
add_unittest(${SRCS})

