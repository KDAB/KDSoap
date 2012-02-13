KDWSDL_OPTIONS = -server
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_wsdl_document.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

unix:PRE_TARGETDEPS = $${TOP_BUILD_DIR}/lib/libtesttools.a

KDWSDL = thomas-bayer.wsdl mywsdl_document.wsdl

OTHER_FILES = $$KDWSDL thomas-bayer.xsd
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

