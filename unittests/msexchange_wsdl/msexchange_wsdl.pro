include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
QT += network xml
SOURCES = msexchange_wsdl.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = Services.wsdl
OTHER_FILES += $$KDWSDL
