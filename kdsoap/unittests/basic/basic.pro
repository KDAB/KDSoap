KDSOAP_PATH = $$PWD/../..
include( $$KDSOAP_PATH/unittests/unittests.pri )
SOURCES = basic.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = first
QMAKE_EXTRA_TARGETS += test
