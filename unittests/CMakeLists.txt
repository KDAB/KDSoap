set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(${QT_USE_FILE})

include_directories(../src/ ../src/KDSoapClient/ ../src/KDSoapServer/)

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



