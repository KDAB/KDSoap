# http://tech.ebu.ch/docs/tech/tech3356_FIMS_V1_0_7.zip

KDWSDL_OPTIONS = -server
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_tech3356.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

unix:PRE_TARGETDEPS = $${TOP_BUILD_DIR}/lib/libtesttools.a

KDWSDL = transformMedia-V1_0_7.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

