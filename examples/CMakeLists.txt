find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Widgets CONFIG REQUIRED)
list(APPEND QT_LIBRARIES Qt5::Widgets)

include(../KDSoapMacros.cmake)
include_directories(../src/ ../src/KDSoapClient/ ../src/KDSoapServer/)

add_subdirectory(helloworld_client)
add_subdirectory(helloworld_server)
add_subdirectory(bank_gui)
add_subdirectory(bank_wsdl)
