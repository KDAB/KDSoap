# This would apply to all .wsdl files... qmake is too limited for this.
# KDWSDL_OPTIONS = -service OrteLookup

include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = wsaddressingtest.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = wsaddressing.wsdl # Find one relevant on the internet
