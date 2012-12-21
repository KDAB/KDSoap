TEMPLATE = app
TARGET = kdwsdl2cpp
SOURCES = compiler.cpp \
    converter.cpp \
    converter_clientstub.cpp \
    converter_complextype.cpp \
    converter_serverstub.cpp \
    converter_simpletype.cpp \
    creator.cpp \
    main.cpp \
    namemapper.cpp \
    settings.cpp \
    typemap.cpp
HEADERS = compiler.h \
    converter.h
INCLUDEPATH += $${TOP_SOURCE_DIR}/kdwsdl2cpp
QT -= gui
QT += xml

CONFIG += console
macx:CONFIG -= app_bundle

# Relink when a static lib changed
unix:PRE_TARGETDEPS += $${TOP_BUILD_DIR}/lib/libkode.a \
    $${TOP_BUILD_DIR}/lib/libwsdl.a \
    $${TOP_BUILD_DIR}/lib/libxmlschema.a \
    $${TOP_BUILD_DIR}/lib/libxmlcommon.a
win32-msvc*:PRE_TARGETDEPS += $${TOP_BUILD_DIR}/lib/kode.lib \
    $${TOP_BUILD_DIR}/lib/wsdl.lib \
    $${TOP_BUILD_DIR}/lib/xmlschema.lib \
    $${TOP_BUILD_DIR}/lib/xmlcommon.lib
LIBS += -L$${TOP_BUILD_DIR}/lib \
    -lkode \
    -lwsdl \
    -lxmlschema \
    -lxmlcommon
include($${TOP_SOURCE_DIR}/variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII \
    QBA_NO_CAST_TO_VOID \
    QBA_NO_CAST_TO_ASCII \
    QT_NO_CAST_FROM_ASCII

#installation targets
target.path = $$INSTALL_PREFIX/bin

INSTALLS += target
