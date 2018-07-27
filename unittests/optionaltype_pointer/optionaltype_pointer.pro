QT       -= gui

KDWSDL_OPTIONS = -optional-element-type raw-pointer

include(../unittests.pri)

CONFIG   += console

TEMPLATE = app

SOURCES += \
    testpointerapi.cpp

HEADERS += \
    testpointerapi.h

KDWSDL = test.wsdl

OTHER_FILES += \
    kdsoap.pri \
    test.wsdl

test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test
