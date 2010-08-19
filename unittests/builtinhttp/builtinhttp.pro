KDSOAP_PATH = $$PWD/../..
include( $$KDSOAP_PATH/unittests/unittests.pri )
QT += network xml
SOURCES = builtinhttp.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = sugarcrm.wsdl
