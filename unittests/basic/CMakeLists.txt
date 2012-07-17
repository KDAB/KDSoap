project(basic)

set(basic_SRCS basic.cpp)

include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
SOURCES = basic.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = first
QMAKE_EXTRA_TARGETS += test

