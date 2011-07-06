KDSOAP_PATH = $$PWD/../..

WSDL_DIR = generated

include( $$KDSOAP_PATH/examples/examples.pri )

SOURCES = main.cpp
HEADERS += helloworld_client.h

KDWSDL = helloworld.wsdl

CONFIG -= console
QT += gui
macx:CONFIG += app_bundle

