include( $${TOP_SOURCE_DIR}/unittests/unittests.pri )
include($${TOP_SOURCE_DIR}/variables.pri)
QT += network xml
SOURCES = test_list_restriction.cpp\
../../kdwsdl2cpp/src/compiler.cpp\
../../kdwsdl2cpp/src/converter.cpp \
../../kdwsdl2cpp/src/converter_clientstub.cpp \
../../kdwsdl2cpp/src/converter_complextype.cpp \
../../kdwsdl2cpp/src/converter_serverstub.cpp \
../../kdwsdl2cpp/src/converter_simpletype.cpp \
../../kdwsdl2cpp/src/creator.cpp \
../../kdwsdl2cpp/src/namemapper.cpp \
../../kdwsdl2cpp/src/settings.cpp \
../../kdwsdl2cpp/src/typemap.cpp \
../../kdwsdl2cpp/src/elementargumentserializer.cpp
test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = list_restriction.wsdl

OTHER_FILES = $$KDWSDL
LIBS        += -L$${TOP_BUILD_DIR}/lib\
    -lkode \
    -lwsdl \
    -lxmlschema \
    -lxmlcommon
INCLUDEPATH += $${TOP_SOURCE_DIR}/kdwsdl2cpp\
$${TOP_SOURCE_DIR}/kdwsdl2cpp/common\
$${TOP_SOURCE_DIR}/kdwsdl2cpp/libkode\
$${TOP_SOURCE_DIR}/kdwsdl2cpp/schema\
$${TOP_SOURCE_DIR}/kdwsdl2cpp/wsdl\
$${TOP_SOURCE_DIR}/kdwsdl2cpp/src
