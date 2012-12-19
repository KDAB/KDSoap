WSDL_DIR = generated

include( $${TOP_SOURCE_DIR}/examples/examples.pri )

SOURCES = main.cpp
HEADERS += helloworld_client.h

KDWSDL = helloworld.wsdl

CONFIG -= console

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
} else {
    QT += gui
}

macx:CONFIG += app_bundle

