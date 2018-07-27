QT       -= gui

KDWSDL_OPTIONS = -optional-element-type boost-optional

include(../unittests.pri)

CONFIG   += console

TEMPLATE = app

SOURCES += \
    testboostapi.cpp

HEADERS += \
    testboostapi.h

KDWSDL = test.wsdl

OTHER_FILES += \
    kdsoap.pri \
    test.wsdl

test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test
