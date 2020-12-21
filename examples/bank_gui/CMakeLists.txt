project(bank_gui)

set(bank_gui_SRCS bank_gui.cpp mainwindow.cpp)

KDSOAP_GENERATE_WSDL(bank_gui_SRCS BLZService.wsdl)

add_executable(bank_gui ${bank_gui_SRCS} resources.qrc)
target_link_libraries(bank_gui ${QT_LIBRARIES} kdsoap)
