## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2012-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} COMPONENTS Test CONFIG REQUIRED)
list(APPEND QT_LIBRARIES Qt${QT_MAJOR_VERSION}::Test)

include_directories(../src/ ../src/KDSoapClient/ ../src/KDSoapServer/ ../testtools/ ../kdwsdl2cpp/ ../kdwsdl2cpp/src/ ../kdwsdl2cpp/libkode/code_generation/ ../kdwsdl2cpp/libkode/ ../kdwsdl2cpp/libkode/schema/ ../kdwsdl2cpp/wsdl/)
include(${CMAKE_BINARY_DIR}/KDSoap/KDSoapMacros.cmake)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(add_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)

  if(WSDL_FILES)
    if(NOT DEFINED KSWSDL2CPP_OPTION)
      set(KSWSDL2CPP_OPTION -use-local-files-only)
    else()
      set(KSWSDL2CPP_OPTION ${KSWSDL2CPP_OPTION} -use-local-files-only)
    endif()
    kdsoap_generate_wsdl(_test ${WSDL_FILES})
  endif()

  add_executable(${_name} ${_source} ${_test})

  add_test(NAME kdsoap-${_name} COMMAND ${_name})
  target_link_libraries(${_name} ${QT_LIBRARIES} kdsoap testtools)
  if(EXTRA_LIBS)
    target_link_libraries(${_name} ${EXTRA_LIBS})
  endif()
endmacro()

if(MSVC)
  #with msvc, cribis requires the /bigobj option
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

add_subdirectory(basic)
add_subdirectory(builtinhttp)
add_subdirectory(wsdl_rpc)
add_subdirectory(wsdl_rpc-server)
add_subdirectory(sugar_wsdl)
add_subdirectory(ihc_wsdl)
add_subdirectory(salesforce_wsdl)
add_subdirectory(groupwise_wsdl)
add_subdirectory(logbook_wsdl)
add_subdirectory(messagereader)
add_subdirectory(servertest)
add_subdirectory(msexchange_noservice_wsdl)
add_subdirectory(msexchange_wsdl)
add_subdirectory(multiple_input_param)
add_subdirectory(wsdl_document)
add_subdirectory(dwservice_wsdl)
add_subdirectory(dwservice_12_wsdl)
add_subdirectory(dwservice_combined_wsdl)
add_subdirectory(tech3356_wsdl)
add_subdirectory(clearbooks)
add_subdirectory(cribis)
add_subdirectory(uitapi)
add_subdirectory(empty_response_wsdl)
add_subdirectory(element_ns_wsdl)
add_subdirectory(specialchars_wsdl)
add_subdirectory(optionaltype_regular)
add_subdirectory(optionaltype_pointer)
add_subdirectory(enum_escape)
add_subdirectory(enum_with_length_restriction)
add_subdirectory(soap12)
add_subdirectory(literal_true_false)
add_subdirectory(import_definition)
add_subdirectory(unqualified_formdefault)
add_subdirectory(encapsecurity)
add_subdirectory(prefix_wsdl)
add_subdirectory(vidyo)
add_subdirectory(ws_addressing_support)
add_subdirectory(ws_usernametoken_support)
add_subdirectory(empty_element_wsdl)
add_subdirectory(ws_discovery_wsdl)
add_subdirectory(soap_over_udp)

# These need internet access
add_subdirectory(webcalls)
add_subdirectory(webcalls_wsdl)

if(BOOST_OPTIONAL_DIR)
  add_subdirectory(optionaltype_boost_optional)
  add_subdirectory(default_attribute_value_wsdl)
endif()

add_subdirectory(test_calc)
add_subdirectory(dv_terminalauth)
add_subdirectory(date_example)
add_subdirectory(enzo)
add_subdirectory(fault_namespace)
add_subdirectory(empty_list_wsdl)

add_subdirectory(kddatetime)

add_subdirectory(QSharedPointer_include)
add_subdirectory(pki.pca.dfn.de)
