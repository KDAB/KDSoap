KDSOAP_PATH = $$PWD/../..

include( $$KDSOAP_PATH/examples/examples.pri )

QT -= gui
macx:CONFIG -= app_bundle

HEADERS =
SOURCES = holidays.cpp
