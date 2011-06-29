KDSOAP_PATH = $$(KDSOAPDIR)
include( $$KDSOAP_PATH/kdsoap.pri )

KDWSDL = WSDL/IntegrationTest.wsdl
SOURCES = main.cpp
CONFIG += qtestlib

