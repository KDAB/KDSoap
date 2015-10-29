include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
TARGET = vidyo
QT += network xml
SOURCES = test_vidyo.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = VidyoPortalGuestService.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib

