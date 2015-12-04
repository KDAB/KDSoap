project( holidays_gui )

include_directories(${QT_QTGUI_INCLUDE_DIR})

set(holidays_gui_SRCS holidays_gui.cpp mainwindow.cpp)

KDSOAP_GENERATE_WSDL(holidays_gui_SRCS holidays.wsdl)

qt4_add_resources(holidays_gui_SRCS resources.qrc)

add_executable(holidays_gui ${holidays_gui_SRCS})
target_link_libraries(holidays_gui ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTWIDGETS_LIBRARY} kdsoap)
