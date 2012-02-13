KDWSDL_OPTIONS = -server
WSDL_DIR = generated

include( $${TOP_SOURCE_DIR}/examples/examples.pri )

SOURCES = main.cpp
HEADERS = helloworld_server.h

KDWSDL = helloworld.wsdl

LIBS += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB
