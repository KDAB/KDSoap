KDWSDL_OPTIONS = -server

QT       -= gui

include(../unittests.pri)

CONFIG   += console

TEMPLATE = app

SOURCES += \
    test_soap12.cpp

HEADERS +=

KDWSDL = soap12.wsdl
OTHER_FILES += soap12.wsdl

LIBS        += -L$${TOP_BUILD_DIR}/lib -l$$KDSOAPSERVERLIB

test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test
