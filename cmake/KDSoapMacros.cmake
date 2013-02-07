macro( KDSOAP_GENERATE_WSDL _sources )
  set(KDWSDL2CPP kdwsdl2cpp )
  if(KDSoap_KDWSDL2CPP_EXE)
    set(KDWSDL2CPP ${KDSoap_KDWSDL2CPP_EXE})
  endif()
  set(_KSWSDL2CPP_OPTION)
  if(KSWSDL2CPP_OPTION)
     set(_KSWSDL2CPP_OPTION ${KSWSDL2CPP_OPTION})
  endif(KSWSDL2CPP_OPTION)
  foreach (_source_FILE ${ARGN})
    get_filename_component(_tmp_FILE ${_source_FILE} ABSOLUTE)
    get_filename_component(_basename ${_tmp_FILE} NAME_WE)
    set(_header_wsdl_FILE ${CMAKE_CURRENT_BINARY_DIR}/wsdl_${_basename}.h)
    set(_source_wsdl_FILE ${CMAKE_CURRENT_BINARY_DIR}/wsdl_${_basename}.cpp)
    add_custom_command(OUTPUT ${_header_wsdl_FILE} 
       COMMAND ${KDWSDL2CPP}
       ARGS ${_KSWSDL2CPP_OPTION} ${_tmp_FILE} -o ${_header_wsdl_FILE}
       MAIN_DEPENDENCY ${_tmp_FILE}
       DEPENDS ${_tmp_FILE} ${KDWSDL2CPP} )

    add_custom_command(OUTPUT ${_source_wsdl_FILE} 
       COMMAND ${KDWSDL2CPP}
       ARGS ${_KSWSDL2CPP_OPTION} -impl ${_header_wsdl_FILE} ${_tmp_FILE} -o ${_source_wsdl_FILE}
       MAIN_DEPENDENCY ${_tmp_FILE} ${_header_wsdl_FILE}
       DEPENDS ${_tmp_FILE} ${KDWSDL2CPP} )

    qt4_wrap_cpp(_sources_MOCS ${_header_wsdl_FILE} ) 
    list(APPEND ${_sources} ${_header_wsdl_FILE} ${_source_wsdl_FILE} ${_sources_MOCS})
  endforeach (_source_FILE)
endmacro(KDSOAP_GENERATE_WSDL _sources )

