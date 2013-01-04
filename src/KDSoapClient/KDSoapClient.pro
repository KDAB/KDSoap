TEMPLATE = lib
TARGET = kdsoap
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui
QT += network

# Workaround for visual studio integration
DESTDIR = $${TOP_BUILD_DIR}/lib
win32:DLLDESTDIR = $${TOP_BUILD_DIR}/bin
include($${TOP_SOURCE_DIR}/variables.pri)

# Only used by Mac frameworks.
# See the include subdir for standard header installation
# TODO: install these from include/ as well
INSTALLHEADERS = KDSoapMessage.h \
    KDSoapClientInterface.h \
    KDSoapPendingCall.h \
    KDSoapPendingCallWatcher.h \
    KDSoapValue.h \
    KDSoapGlobal.h \
    KDSoapJob.h \
    KDSoapAuthentication.h \
    KDSoapNamespaceManager.h \
    KDSoapSslHandler.h \
    KDDateTime.h
PRIVATEHEADERS = KDSoapPendingCall_p.h \
    KDSoapPendingCallWatcher_p.h \
    KDSoapClientInterface_p.h \
    KDSoapClientThread_p.h \
    KDSoapMessageReader_p.h \
    KDSoapMessageWriter_p.h \
    KDSoapNamespacePrefixes_p.h

HEADERS = $$INSTALLHEADERS \
    $$PRIVATEHEADERS

SOURCES = KDSoapMessage.cpp \
    KDSoapClientInterface.cpp \
    KDSoapPendingCall.cpp \
    KDSoapPendingCallWatcher.cpp \
    KDSoapClientThread.cpp \
    KDSoapValue.cpp \
    KDSoapAuthentication.cpp \
    KDSoapNamespaceManager.cpp \
    KDSoapMessageReader.cpp \
    KDSoapMessageWriter.cpp \
    KDSoapNamespacePrefixes.cpp \
    KDDateTime.cpp \
    KDSoapJob.cpp \
    KDSoapSslHandler.cpp
DEFINES += KDSOAP_BUILD_KDSOAP_LIB

# installation targets:
target.path = $$INSTALL_PREFIX/lib
INSTALLS += target

# Mac frameworks
macx:lib_bundle: { 
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$INSTALLHEADERS
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}
