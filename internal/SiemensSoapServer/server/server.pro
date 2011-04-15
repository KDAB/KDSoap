KDSOAP_PATH = $$(KDSOAPDIR)
KDWSDL_OPTIONS = -server
include( $$KDSOAP_PATH/kdsoap.pri )

KDWSDL = ../IPGW_V1.0.wsdl

SOURCES = server.cpp
LIBS += -l$$KDSOAPSERVERLIB

