KDWSDL_OPTIONS = -server
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_wsdl_rpc_server.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

unix:PRE_TARGETDEPS = $${TOP_BUILD_DIR}/lib/libtesttools.a

KDWSDL = rpcexample.wsdl sayhello.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

