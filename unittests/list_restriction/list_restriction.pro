include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )

QT += network xml
SOURCES = test_list_restriction.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test


LIBS += -L$${TOP_BUILD_DIR}/lib $${KDWSDL2CPP_LIBS}
unix:PRE_TARGETDEPS += $${TOP_BUILD_DIR}/lib/libkdwsdl2cpp_lib.a
