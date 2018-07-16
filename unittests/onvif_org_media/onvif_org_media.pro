KDWSDL_OPTIONS = -server -namespace media -import-path $${TOP_SOURCE_DIR}/unittests/onvif.org -use-local-files-only
include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_onvif_media.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = media.wsdl

OTHER_FILES = $$KDWSDL *.xsd
LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

