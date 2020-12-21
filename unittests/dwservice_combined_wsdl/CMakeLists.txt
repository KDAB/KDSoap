project(dwservice_combined_wsdl)
set(KSWSDL2CPP_OPTION -namespace KDAB)

set(WSDL_FILES DWServiceCombined.wsdl)
set(dwservice_combined_wsdl_SRCS test_dwservice_combined_wsdl.cpp )

add_unittest(${dwservice_combined_wsdl_SRCS} )
