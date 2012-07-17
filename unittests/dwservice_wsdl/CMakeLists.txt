project(dwservice_wsdl)
set(KSWSDL2CPP_OPTION -namespace KDAB)

set(WSDL_FILES DWService.wsdl)
set(dwservice_wsdl_SRCS test_dwservice_wsdl.cpp )
set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${dwservice_wsdl_SRCS} )
