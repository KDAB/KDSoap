TEMPLATE = lib
TARGET = kdsoap-server
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui

QT += network

# Workaround for visual studio integration
DESTDIR = $${TOP_BUILD_DIR}/lib
win32:DLLDESTDIR = $${TOP_BUILD_DIR}/bin

include($${TOP_SOURCE_DIR}/variables.pri)
INSTALLHEADERS = KDSoapServer.h \
                 KDSoapServerAuthInterface.h \
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
    KDSoapServerAuthInterface.h \
    KDSoapServerObjectInterface.h \
    KDSoapDelayedResponseHandle.h

SOURCES = KDSoapServer.cpp \
    KDSoapThreadPool.cpp \
    KDSoapServerSocket.cpp \
    KDSoapServerThread.cpp \
    KDSoapSocketList.cpp \
    KDSoapServerAuthInterface.cpp \
    KDSoapServerObjectInterface.cpp \
    KDSoapDelayedResponseHandle.cpp

DEFINES += KDSOAP_BUILD_KDSOAPSERVER_LIB

# We use the soap client library, for xml parsing
INCLUDEPATH += . $${TOP_SOURCE_DIR}/src/KDSoapClient
DEPENDPATH += . $${TOP_SOURCE_DIR}/src//KDSoapClient
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


