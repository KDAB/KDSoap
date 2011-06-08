KDSOAP_PATH = $$PWD/../..

include( $$KDSOAP_PATH/examples/examples.pri )

SOURCES = main.cpp

KDWSDL = helloworld.wsdl

#CONFIG -= console
#QT += gui
#macx:CONFIG += app_bundle
