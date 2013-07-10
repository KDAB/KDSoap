KDWSDL_OPTIONS = -namespace KDAB

include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = test_dwservice_12_wsdl.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test


KDWSDL = DWService_12.wsdl
OTHER_FILES += $$KDWSDL
