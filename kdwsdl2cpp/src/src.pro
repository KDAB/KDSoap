TEMPLATE = app
TARGET = kdwsdl2cpp
SOURCES = compiler.cpp \
    converter.cpp \
    converter_attribute.cpp \
    converter_clientstub.cpp \
    converter_complextype.cpp \
    converter_serverstub.cpp \
    converter_simpletype.cpp \
    converter_utils.cpp \
    creator.cpp \
    main.cpp \
    namemapper.cpp \
    settings.cpp \
    typemap.cpp
HEADERS = compiler.h \
    converter.h
INCLUDEPATH += ..
QT -= gui
QT += xml

macx:CONFIG -= app_bundle

# Relink when a static lib changed
unix:PRE_TARGETDEPS += ../../lib/libkode.a \
    ../../lib/libwsdl.a \
    ../../lib/libxmlschema.a \
    ../../lib/libxmlcommon.a
LIBS += -L../../lib \
    -lkode \
    -lwsdl \
    -lxmlschema \
    -lxmlcommon
include(../../variables.pri)
DEFINES -= QT_NO_CAST_TO_ASCII \
    QBA_NO_CAST_TO_VOID \
    QBA_NO_CAST_TO_ASCII \
    QT_NO_CAST_FROM_ASCII

#installation targets
target.path = $$INSTALL_PREFIX/bin

INSTALLS += target
