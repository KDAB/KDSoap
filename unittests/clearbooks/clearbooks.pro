KDWSDL_OPTIONS = -server
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
TARGET = clearbooks
QT += network xml
SOURCES = test_clearbooks.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = clearbooks.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

