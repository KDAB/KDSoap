include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
include($${TOP_SOURCE_DIR}/variables.pri)
QT += network xml
SOURCES = test_list_restriction.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = list_restriction.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib\
    -lkode \
    -lwsdl \
    -lxmlschema \
    -lxmlcommon
INCLUDEPATH += $${TOP_SOURCE_DIR}/kdwsdl2cpp
