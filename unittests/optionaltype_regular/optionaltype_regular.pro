QT       -= gui

include(../unittests.pri)

CONFIG   += console

TEMPLATE = app

SOURCES += \
    testregularapi.cpp

HEADERS += \
    testregularapi.h

KDWSDL_OPTIONS = -namespace KDAB
KDWSDL = test.wsdl

OTHER_FILES += \
    kdsoap.pri \
    test.wsdl

test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test
