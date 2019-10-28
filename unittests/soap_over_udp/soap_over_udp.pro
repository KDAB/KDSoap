KDWSDL_OPTIONS = -use-local-files-only
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += xml
SOURCES = test_soap_over_udp.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = wsdd-discovery-200901.wsdl
