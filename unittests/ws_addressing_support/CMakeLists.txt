project(ws_addressing_support)

set(WSDL_FILES wsaddressing.wsdl)
set(ws_addressing_support_SRCS wsaddressingtest.cpp )

set(EXTRA_LIBS ${QT_QTXML_LIBRARY})

add_unittest(${ws_addressing_support_SRCS} )
