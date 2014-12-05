KDWSDL_OPTIONS = -server
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
TARGET = encapsecurity
QT += network xml
SOURCES = test_encapsecurity.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

# From http://demo.encapsecurity.com/pt/encap-ws/auth/stateless?wsdl
KDWSDL = authstateless.wsdl

OTHER_FILES = $$KDWSDL *.xsd
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

