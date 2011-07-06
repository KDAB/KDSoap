KDSOAP_PATH = $$PWD/../..
KDWSDL_OPTIONS = -server
WSDL_DIR = generated

include( $$KDSOAP_PATH/examples/examples.pri )

SOURCES = main.cpp
HEADERS = helloworld_server.h

KDWSDL = helloworld.wsdl

LIBS += -L$$KDSOAP_PATH/lib -l$$KDSOAPSERVERLIB
