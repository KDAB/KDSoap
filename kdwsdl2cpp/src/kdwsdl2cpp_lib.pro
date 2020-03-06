TEMPLATE = lib
TARGET = $${KDWSDL2CPP_LIB}
SOURCES = compiler.cpp \
    converter.cpp \
    converter_clientstub.cpp \
    converter_complextype.cpp \
    converter_serverstub.cpp \
    converter_simpletype.cpp \
    creator.cpp \
    namemapper.cpp \
    settings.cpp \
    typemap.cpp \
    elementargumentserializer.cpp
HEADERS = compiler.h \
    converter.h \
    elementargumentserializer.h
INCLUDEPATH += $${TOP_SOURCE_DIR}/kdwsdl2cpp $${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode
QT -= gui
QT += xml

CONFIG += staticlib

CONFIG += console
static:macx:QMAKE_LFLAGS += -Wl,-rpath,$$[QT_INSTALL_LIBS]

# Relink when a static lib changed
unix:PRE_TARGETDEPS += $${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/libkode/libkode.a \
    $${TOP_BUILD_DIR}/lib/libwsdl.a \
    $${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/schema/libxmlschema.a \
    $${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/common/libxmlcommon.a
win32-msvc*:PRE_TARGETDEPS += $${TOP_BUILD_DIR}/lib/kode.lib \
    $${TOP_BUILD_DIR}/lib/wsdl.lib \
    $${TOP_BUILD_DIR}/lib/xmlschema.lib \
    $${TOP_BUILD_DIR}/lib/xmlcommon.lib
LIBS += -L$${TOP_BUILD_DIR}/lib \
        -L$${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/libkode \
        -L$${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/schema \
        -L$${TOP_BUILD_DIR}/kdwsdl2cpp/libkode/common \
    -lkode \
    -lwsdl \
    -lxmlschema \
    -lxmlcommon
include($${TOP_SOURCE_DIR}/variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII \
    QBA_NO_CAST_TO_VOID \
    QBA_NO_CAST_TO_ASCII \
    QT_NO_CAST_FROM_ASCII
