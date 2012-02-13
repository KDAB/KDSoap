include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = builtinhttp.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

LIBS        += -L$$$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB
