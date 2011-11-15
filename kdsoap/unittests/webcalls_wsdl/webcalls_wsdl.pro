# This would apply to all .wsdl files... qmake is too limited for this.
# KDWSDL_OPTIONS = -service OrteLookup

KDSOAP_PATH = $$PWD/../..
include( $$KDSOAP_PATH/unittests/unittests.pri )
QT += network xml
SOURCES = webcalls_wsdl.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = soapresponder.wsdl holidays.wsdl BFGlobalService.wsdl OrteLookup.wsdl
