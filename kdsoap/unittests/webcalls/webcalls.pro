KDSOAP_PATH = $$PWD/../..

include( $$KDSOAP_PATH/unittests/unittests.pri )

QT += network

HEADERS =
SOURCES = webcalls.cpp

test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

