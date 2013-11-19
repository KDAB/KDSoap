include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_import_definition.cpp

test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

#KDWSDL = import_definition_homemade.wsdl
KDWSDL = import_definition.wsdl
OTHER_FILES += $$KDWSDL

