set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTXML TRUE)
include(${QT_USE_FILE})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCES
	common/fileprovider.cpp
	common/messagehandler.cpp
	common/nsmanager.cpp
	common/parsercontext.cpp
	common/qname.cpp
	libkode/code.cpp
	libkode/enum.cpp
	libkode/style.cpp
	libkode/printer.cpp
	libkode/license.cpp
	libkode/file.cpp
	libkode/class.cpp
	libkode/function.cpp
	libkode/variable.cpp
	libkode/membervariable.cpp
	libkode/typedef.cpp
	libkode/statemachine.cpp
	schema/annotation.cpp
	schema/attribute.cpp
	schema/complextype.cpp
	schema/element.cpp
	schema/parser.cpp
	schema/simpletype.cpp
	schema/types.cpp
	schema/xmlelement.cpp
	schema/xsdtype.cpp
	schema/attributegroup.cpp
	schema/compositor.cpp
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
	src/main.cpp
	src/namemapper.cpp
	src/settings.cpp
	src/typemap.cpp
)

add_executable(kdwsdl2cpp ${SOURCES})
target_link_libraries(kdwsdl2cpp ${QT_LIBRARIES})