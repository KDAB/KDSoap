#this is the option given to KDWSDL2CPP when generating cpp from wsdl
KDWSDL_OPTIONS = -keep-unused-types

include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )

QT += network xml
SOURCES = keep_unused_types.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = keep_unused_types.wsdl
