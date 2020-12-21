project(wsdl_document)
set(KSWSDL2CPP_OPTION -server)

set(WSDL_FILES thomas-bayer.wsdl mywsdl_document.wsdl)
set(wsdl_document_SRCS test_wsdl_document.cpp )
set(EXTRA_LIBS kdsoap-server)

add_unittest(${wsdl_document_SRCS} )
