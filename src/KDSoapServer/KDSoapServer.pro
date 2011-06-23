TEMPLATE = lib
TARGET = kdsoap-server
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui

QT += network

# Workaround for visual studio integration
DESTDIR = ../../lib
win32:DLLDESTDIR = ../../bin

include(../variables.pri)
INSTALLHEADERS = KDSoapServer.h \
                 KDSoapServerObjectInterface.h \
                 KDSoapServerGlobal.h

EXTENSIONLESSHEADERS =
PRIVATEHEADERS =

HEADERS = $$INSTALLHEADERS \
    $$PRIVATEHEADERS \
    KDSoapThreadPool.h \
    KDSoapServerSocket_p.h \
    KDSoapServerThread_p.h \
    KDSoapSocketList_p.h \
    KDSoapServerObjectInterface.h

SOURCES = KDSoapServer.cpp \
    KDSoapThreadPool.cpp \
    KDSoapServerSocket.cpp \
    KDSoapServerThread.cpp \
    KDSoapSocketList.cpp \
    KDSoapServerObjectInterface.cpp

DEFINES += KDSOAP_BUILD_KDSOAPSERVER_LIB

# We use the soap client library, for xml parsing
INCLUDEPATH += . ../KDSoapClient
DEPENDPATH += . ../KDSoapClient
LIBS        += -L$$DESTDIR -l$$KDSOAPLIB

# installation targets:
headers.files = $$INSTALLHEADERS \
    $$EXTENSIONLESSHEADERS
headers.path = $$INSTALL_PREFIX/include
#INSTALLS += headers

target.path = $$INSTALL_PREFIX/lib
INSTALLS += target

# Mac frameworks
macx:lib_bundle: {
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$INSTALLHEADERS \
        $$EXTENSIONLESSHEADERS
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}
