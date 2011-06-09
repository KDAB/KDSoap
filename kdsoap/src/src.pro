TEMPLATE = lib
TARGET = kdsoap
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui

QT += network

# Workaround for visual studio integration
DESTDIR = ../lib
win32:DLLDESTDIR = ../bin
include(../variables.pri)
INSTALLHEADERS = KDSoapMessage.h \
    KDSoapClientInterface.h \
    KDSoapPendingCall.h \
    KDSoapPendingCallWatcher.h \
    KDSoapValue.h \
    KDSoapGlobal.h \
    KDSoapAuthentication.h \
    KDSoapNamespaceManager.h \
    KDDateTime.h
EXTENSIONLESSHEADERS = KDSoap
PRIVATEHEADERS = KDSoapPendingCall_p.h \
    KDSoapPendingCallWatcher_p.h \
    KDSoapClientInterface_p.h \
    KDSoapClientThread_p.h \

HEADERS = $$INSTALLHEADERS \
    $$PRIVATEHEADERS \
    KDSoapMessageWriter_p.h \
    KDSoapNamespacePrefixes_p.h
SOURCES = KDSoapMessage.cpp \
    KDSoapClientInterface.cpp \
    KDSoapPendingCall.cpp \
    KDSoapPendingCallWatcher.cpp \
    KDSoapClientThread.cpp \
    KDSoapValue.cpp \
    KDSoapAuthentication.cpp \
    KDSoapNamespaceManager.cpp \
    KDSoapMessageWriter.cpp \
    KDSoapNamespacePrefixes.cpp \
    KDDateTime.cpp
DEFINES += KDSOAP_BUILD_KDSOAP_LIB

# installation targets:
headers.files = $$INSTALLHEADERS \
    $$EXTENSIONLESSHEADERS
headers.path = $$INSTALL_PREFIX/include
INSTALLS += headers

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
