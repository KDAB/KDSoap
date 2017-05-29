include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
TARGET = test_calc
QT += network xml
SOURCES = test_calc.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = calc.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib
