set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTXML TRUE)

include(${QT_USE_FILE})

include_directories(../src/ ../src/KDSoapClient/ ../src/KDSoapServer/ ../testtools/)
include(../KDSoapMacros.cmake)

macro(add_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)

  qt4_wrap_cpp(_mocs ${_source} )
  if(WSDL_FILES)
    KDSOAP_GENERATE_WSDL(_test ${WSDL_FILES} )
  endif(WSDL_FILES)
  set_property(SOURCE ${_source} PROPERTY OBJECT_DEPENDS ${_mocs})



  add_executable(${_name} ${_source} ${_test})

  add_test(NAME kdsoap-${_name} COMMAND ${_name})
  target_link_libraries(${_name} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} kdsoap testtools)
  if(EXTRA_LIBS)
    target_link_libraries(${_name} ${EXTRA_LIBS} )
  endif(EXTRA_LIBS)
endmacro()

add_subdirectory(basic)
add_subdirectory(builtinhttp)
add_subdirectory(dwservice_12_wsdl)
add_subdirectory(dwservice_combined_wsdl)
add_subdirectory(dwservice_wsdl)
add_subdirectory(groupwise_wsdl)
add_subdirectory(logbook_wsdl)
add_subdirectory(messagereader)
add_subdirectory(msexchange_wsdl)
add_subdirectory(salesforce_wsdl)
add_subdirectory(servertest)
add_subdirectory(sugar_wsdl)
add_subdirectory(webcalls)
add_subdirectory(webcalls_wsdl)
add_subdirectory(wsdl_document)
add_subdirectory(wsdl_rpc)



