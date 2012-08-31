project(KDSoap CXX)
cmake_minimum_required(VERSION 2.8.7)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_MODULE_PATH}")

option(${PROJECT_NAME}_STATIC "Build statically" OFF)
option(${PROJECT_NAME}_TESTS "Build the tests" OFF)

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 0)
set(${PROJECT_NAME}_VERSION_PATCH 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# setup default install locations
include(InstallLocation)

find_package(Qt4 4.7 QUIET REQUIRED QtCore QtMain QtGui)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC TRUE)

add_definitions(-DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)
add_definitions(-DUSE_EXCEPTIONS -DQT_FATAL_ASSERT)

if(${PROJECT_NAME}_STATIC)
	set(${PROJECT_NAME}_LIBRARY_MODE "STATIC")
else()
	set(${PROJECT_NAME}_LIBRARY_MODE "SHARED")
endif()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
	set(${PROJECT_NAME}_IS_ROOT_PROJECT TRUE)

	if(CMAKE_BUILD_TYPE MATCHES "Release")
		add_definitions(-DNDEBUG)
	endif()

	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
	endif()

	if(UNIX AND CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
		set(CMAKE_INSTALL_PREFIX "/usr/local/KDAB/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
	elseif(WIN32)
		set(CMAKE_INSTALL_PREFIX "C:\\KDAB\\${PROJECT_NAME}-$$VERSION")
	endif()

	message(STATUS "Building ${PROJECT_NAME} ${${PROJECT_NAME}_VERSION} in ${CMAKE_BUILD_TYPE} mode")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

install(FILES LICENSE.GPL.txt LICENSE.US.txt LICENSE.txt README.txt kdsoap.pri kdwsdl2cpp.pri DESTINATION .)

add_subdirectory(src)
add_subdirectory(kdwsdl2cpp)
if(${PROJECT_NAME}_IS_ROOT_PROJECT)
	add_subdirectory(features)
	if(${PROJECT_NAME}_TESTS)
		enable_testing()
		add_subdirectory(testtools)
		add_subdirectory(unittests)
	endif()
	add_subdirectory(examples)
endif()

# config files for build tree
set(KDSoap_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(KDSoap_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
set(KDSoap_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(KDSoap_TARGETS_DIR ${CMAKE_CURRENT_BINARY_DIR})
configure_file(KDSoapConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/KDSoapConfig.cmake @ONLY)
configure_file(${KDSoap_CMAKE_DIR}/KDSoapConfigVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/KDSoapConfigVersion.cmake @ONLY)

# config files for install tree
set(KDSoap_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(KDSoap_INCLUDE_DIRS ${KDSoap_INSTALL_PREFIX}/${INSTALL_INCLUDE_DIR})
set(KDSoap_CMAKE_DIR ${KDSoap_INSTALL_PREFIX}/lib/cmake/kdsoap)
set(KDSoap_TARGETS_DIR ${KDSoap_CMAKE_DIR})
configure_file(KDSoapConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/KDSoapConfig.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDSoapConfigVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/KDSoapConfigVersion.cmake @ONLY)

# export targets file in build tree
export(TARGETS kdsoap kdsoap-server
  FILE ${CMAKE_CURRENT_BINARY_DIR}/KDSoapTargets.cmake)

install(FILES
  "${KDSoap_BINARY_DIR}/CMakeFiles/KDSoapConfig.cmake"
  "${KDSoap_BINARY_DIR}/KDSoapConfigVersion.cmake"
  "${KDSoap_SOURCE_DIR}/cmake/KDSoapMacros.cmake"
  DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/kdsoap")

install(EXPORT "KDSoapTargets"
  DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/kdsoap")
