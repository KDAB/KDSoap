TEMPLATE = lib
TARGET = kdsoap
CONFIG(debug, debug|release):!unix:TARGET = $${TARGET}d
QT -= gui

# Workaround for visual studio integration
DESTDIR = ../lib
win32:DLLDESTDIR = ../bin
include(../variables.pri)
INSTALLHEADERS = KDSoapMessage.h \
    KDSoapClientInterface.h \
    KDSoapPendingCall.h \
    KDSoapPendingCallWatcher.h \
    KDSoapValue.h
EXTENSIONLESSHEADERS = 
PRIVATEHEADERS = KDSoapPendingCall_p.h \
    KDSoapPendingCallWatcher_p.h \
    KDSoapMessage_p.h \
    KDSoapClientInterface_p.h \
    KDSoapClientThread_p.h
HEADERS = $$INSTALLHEADERS \
    $$PRIVATEHEADERS
SOURCES = KDSoapMessage.cpp \
    KDSoapClientInterface.cpp \
    KDSoapPendingCall.cpp \
    KDSoapPendingCallWatcher.cpp \
    KDSoapClientThread.cpp
DEFINES += KDSOAP_BUILD_KDSOAP_LIB

# installation targets:
headers.files = $$INSTALLHEADERS \
    $$EXTENSIONLESS_HEADERS
headers.path = $$INSTALL_PREFIX/include
INSTALLS += headers

# install target to install the src code for license holders:
lib.files = $${DESTDIR}
lib.path = $$INSTALL_PREFIX/
INSTALLS += lib

# Mac frameworks
macx:lib_bundle: { 
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$INSTALLHEADERS \
        $$EXTENSIONLESS_HEADERS
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}
