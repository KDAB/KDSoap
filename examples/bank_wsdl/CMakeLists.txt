project( bank_wsdl )

set(bank_wsdl_SRCS bank_wsdl.cpp)

KDSOAP_GENERATE_WSDL(bank_wsdl_SRCS BLZService.wsdl)

add_executable(bank_wsdl ${bank_wsdl_SRCS})
target_link_libraries(bank_wsdl ${QT_LIBRARIES} kdsoap)
