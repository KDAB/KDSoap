#-------------------------------------------------
#
# Project created by QtCreator 2012-08-28T15:13:47
#
#-------------------------------------------------

QT       += core

QT       -= gui

KDWSDL_OPTIONS = -optional-element-type boost-optional

include(../../unittests.pri)

CONFIG   += console
CONFIG   -= app_bundle

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
