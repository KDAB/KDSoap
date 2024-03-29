# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

find_package(
    Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION}
    COMPONENTS Xml CONFIG
    REQUIRED
) #libkode requires Xml
list(APPEND QT_LIBRARIES Qt${QT_VERSION_MAJOR}::Xml)

include_directories(libkode)

remove_definitions(-DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_FOREACH)

set(SOURCES_LIB
    libkode/common/fileprovider.cpp
    libkode/common/messagehandler.cpp
    libkode/common/nsmanager.cpp
    libkode/common/parsercontext.cpp
    libkode/common/qname.cpp
    libkode/code_generation/class.cpp
    libkode/code_generation/code.cpp
    libkode/code_generation/enum.cpp
    libkode/code_generation/license.cpp
    libkode/code_generation/file.cpp
    libkode/code_generation/function.cpp
    libkode/code_generation/include.cpp
    libkode/code_generation/membervariable.cpp
    libkode/code_generation/printer.cpp
    libkode/code_generation/statemachine.cpp
    libkode/code_generation/style.cpp
    libkode/code_generation/typedef.cpp
    libkode/code_generation/variable.cpp
    libkode/schema/annotation.cpp
    libkode/schema/attribute.cpp
    libkode/schema/complextype.cpp
    libkode/schema/element.cpp
    libkode/schema/parser.cpp
    libkode/schema/simpletype.cpp
    libkode/schema/types.cpp
    libkode/schema/xmlelement.cpp
    libkode/schema/xsdtype.cpp
    libkode/schema/attributegroup.cpp
    libkode/schema/compositor.cpp
    libkode/schema/group.cpp
    wsdl/binding.cpp
    wsdl/bindingoperation.cpp
    wsdl/definitions.cpp
    wsdl/element.cpp
    wsdl/fault.cpp
    wsdl/import.cpp
    wsdl/message.cpp
    wsdl/operation.cpp
    wsdl/param.cpp
    wsdl/part.cpp
    wsdl/port.cpp
    wsdl/porttype.cpp
    wsdl/service.cpp
    wsdl/soapbinding.cpp
    wsdl/type.cpp
    wsdl/wsdl.cpp
    src/compiler.cpp
    src/converter.cpp
    src/converter_clientstub.cpp
    src/converter_complextype.cpp
    src/converter_serverstub.cpp
    src/converter_simpletype.cpp
    src/creator.cpp
    src/elementargumentserializer.cpp
    src/namemapper.cpp
    src/settings.cpp
    src/typemap.cpp
)

set(SOURCES_EXE src/main.cpp)

add_library(
    kdwsdl2cpp_lib STATIC
    ${SOURCES_LIB} schemas/schemas.qrc
)
target_link_libraries(
    kdwsdl2cpp_lib ${QT_LIBRARIES}
)

add_executable(
    kdwsdl2cpp
    ${SOURCES_EXE}
)
target_link_libraries(
    kdwsdl2cpp kdwsdl2cpp_lib ${QT_LIBRARIES}
)
set_target_properties(kdwsdl2cpp PROPERTIES OUTPUT_NAME kdwsdl2cpp${${PROJECT_NAME}_LIBRARY_QTID})

install(
    TARGETS kdwsdl2cpp
    EXPORT "KDSoapTargets"
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
    BUNDLE DESTINATION .
)
