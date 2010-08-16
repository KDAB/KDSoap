KDSOAP_PATH = $$PWD/../..

include( $$KDSOAP_PATH/examples/examples.pri )

HEADERS = mainwindow.h
SOURCES = holidays_gui.cpp mainwindow.cpp
RESOURCES = resources.qrc
KDWSDL = holidays.wsdl
