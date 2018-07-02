KDWSDL_OPTIONS = -server -import-path $${TOP_SOURCE_DIR}/unittests/onvif.org -use-local-files-only
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
TARGET = onvif_ptz
QT += network xml
SOURCES = test_onvif_ptz.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = ptz.wsdl

OTHER_FILES = $$KDWSDL *.xsd
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

