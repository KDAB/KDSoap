project(msexchange_wsdl)

set(WSDL_FILES Services.wsdl)
set(msexchange_wsdl_SRCS msexchange_wsdl.cpp )

set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${msexchange_wsdl_SRCS} )
