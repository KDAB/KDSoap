project( holidays_wsdl )


set(holidays_wsdl_SRCS holidays.cpp )

KDSOAP_GENERATE_WSDL(holidays_wsdl_SRCS holidays.wsdl)

add_executable(holidays_wsdl ${holidays_wsdl_SRCS})
target_link_libraries(holidays_wsdl ${QT_QTCORE_LIBRARY} kdsoap )
