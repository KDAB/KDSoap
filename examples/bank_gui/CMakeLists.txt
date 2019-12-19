project(bank_gui)

include_directories(${QT_QTGUI_INCLUDE_DIR})

set(bank_gui_SRCS bank_gui.cpp mainwindow.cpp)

KDSOAP_GENERATE_WSDL(bank_gui_SRCS BLZService.wsdl)

qt4_add_resources(bank_gui_SRCS resources.qrc)

add_executable(bank_gui ${bank_gui_SRCS})
target_link_libraries(bank_gui ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTWIDGETS_LIBRARY} kdsoap)
