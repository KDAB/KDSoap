set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTXML TRUE)
include(${QT_USE_FILE})
include_directories(.. ../src/KDSoapClient)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(testtools_srcs httpserver_p.cpp)

qt4_add_resources(testtools_srcs testtools.qrc)

add_library(testtools STATIC ${testtools_srcs})
