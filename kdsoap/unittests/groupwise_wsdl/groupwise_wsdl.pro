KDSOAP_PATH = $$PWD/../..
include( $$KDSOAP_PATH/unittests/unittests.pri )
QT += network xml
SOURCES = test_groupwise_wsdl.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = groupwise.wsdl
