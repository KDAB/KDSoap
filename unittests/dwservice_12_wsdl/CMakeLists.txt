project(dwservice_12_wsdl)

set(KSWSDL2CPP_OPTION -namespace KDAB)

set(WSDL_FILES DWService_12.wsdl)
set(dwservice_12_wsdl_SRCS test_dwservice_12_wsdl.cpp)

set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${dwservice_12_wsdl_SRCS} )
