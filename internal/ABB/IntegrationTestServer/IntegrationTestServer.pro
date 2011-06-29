QT       += core
QT       += network
QT       += xml

QT       -= gui


TARGET = IntegrationTestServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    WSDL/integrationTestServer.cpp \
    integrationTestService.cpp

HEADERS += \
    WSDL/integrationTestServer.h \
    ServerTestCommons.h \
    integrationTestService.h

KDSOAP_PATH = $$(KDSOAPDIR)
KDWSDL_OPTIONS = -server
include( $$KDSOAP_PATH/kdsoap.pri )

LIBS += -l$$KDSOAPSERVERLIB

INCLUDEPATH += \
            $$KDSOAP_PATH/src \
            $$KDSOAP_PATH/serverlib \
            $$KDSOAP_PATH/include \
            $$KDSOAP_PATH/examples/tools
