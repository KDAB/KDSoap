KDSOAP_PATH = $$PWD/../..
KDWSDL_OPTIONS = -server
include( $$KDSOAP_PATH/unittests/unittests.pri )
QT += network xml
SOURCES = test_wsdl_document.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

unix:PRE_TARGETDEPS = $$KDSOAP_PATH/lib/libtesttools.a

KDWSDL = thomas-bayer.wsdl mywsdl_document.wsdl

OTHER_FILES = $$KDWSDL thomas-bayer.xsd
LIBS        += -L$$KDSOAP_PATH/lib -l$$KDSOAPSERVERLIB

