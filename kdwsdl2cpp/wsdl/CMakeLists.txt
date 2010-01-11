
include_directories( ${kode_SOURCE_DIR} ${kode_SOURCE_DIR}/kwsdl )


########### next target ###############

set(wsdl_LIB_SRCS
   binding.cpp
   bindingoperation.cpp
   definitions.cpp
   element.cpp
   fault.cpp
   import.cpp
   message.cpp
   operation.cpp
   param.cpp
   part.cpp
   port.cpp
   porttype.cpp
   service.cpp
   soapbinding.cpp
   type.cpp
   wsdl.cpp )


kde4_add_library( wsdl SHARED ${wsdl_LIB_SRCS} )

target_link_libraries( wsdl ${KDE4_KDECORE_LIBS} kxmlcommon schema )

set_target_properties( wsdl PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install( TARGETS wsdl ${INSTALL_TARGETS_DEFAULT_ARGS} )
