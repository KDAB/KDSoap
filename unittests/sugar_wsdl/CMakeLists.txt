project(sugar_wsdl)

set(WSDL_FILES sugarcrm.wsdl)
set(sugar_wsdl_SRCS test_sugar_wsdl.cpp)

set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${sugar_wsdl_SRCS} )
