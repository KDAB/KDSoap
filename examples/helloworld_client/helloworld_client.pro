WSDL_DIR = generated

include( $${TOP_SOURCE_DIR}/examples/examples.pri )

SOURCES = main.cpp
HEADERS += helloworld_client.h

KDWSDL = helloworld.wsdl

CONFIG -= console
QT += widgets
macx:CONFIG += app_bundle

