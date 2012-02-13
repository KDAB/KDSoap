include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_sugar_wsdl.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = sugarcrm.wsdl
OTHER_FILES += $$KDWSDL
