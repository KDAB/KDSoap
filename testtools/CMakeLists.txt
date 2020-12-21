find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Xml CONFIG REQUIRED)
list(APPEND QT_LIBRARIES Qt5::Xml)

include_directories(.. ../src/KDSoapClient)

set(testtools_srcs httpserver_p.cpp testtools.qrc)

add_library(testtools STATIC ${testtools_srcs})
target_link_libraries(testtools ${QT_LIBRARIES})
if(Qt5_POSITION_INDEPENDENT_CODE)
  set_target_properties(testtools PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
