KDSOAP_PATH = $$(KDSOAPDIR)
include( $$KDSOAP_PATH/kdsoap.pri )

KDWSDL = ../IPGW_V1.0.wsdl
SOURCES = client.cpp
CONFIG += qtestlib

