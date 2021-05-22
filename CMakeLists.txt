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

# This is the top-level CMakeLists.txt file for the KDSoap project.
#
# Pass the following variables to cmake to control the build:
# (See INSTALL-cmake.txt for more information)
#
# -DKDSoap_QT6=[true|false]
#  Build against Qt6 rather than Qt5
#  Default=false (Qt5 will be used even if Qt6 is available)
#
# -DKDSoap_STATIC=[true|false]
#  Build static libraries
#  Default=false
#
# -DKDSoap_TESTS=[true|false]
#  Build the test harness.
#  Default=false
#
# -DKDSoap_EXAMPLES=[true|false]
#  Build the examples.
#  Default=true
#
# -DKDSoap_DOCS=[true|false]
#  Build the API documentation. Enables the 'docs' build target.
#  Default=false
#

cmake_minimum_required(VERSION 3.0.2)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
if(POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()
if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW) # silence warning due to CMAKE_CXX_VISIBILITY_PRESET
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kdwsdl2cpp/libkode/common")
  message(FATAL_ERROR "Please do git submodule update --init --recursive")
endif()

set(KDAB_INSTALL True)
if((NOT DEFINED ${USE_DEFAULT_INSTALL_LOCATION}) OR (NOT ${USE_DEFAULT_INSTALL_LOCATION}))
  if(NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set(KDAB_INSTALL False)
  endif()
  set(USE_DEFAULT_INSTALL_LOCATION ${KDAB_INSTALL} CACHE INTERNAL "Install to default KDAB Location" FORCE)
endif()

project(KDSoap CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ECM/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDAB/modules")

if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

include(FeatureSummary)

option(${PROJECT_NAME}_STATIC "Build statically" OFF)
option(${PROJECT_NAME}_TESTS "Build the tests" OFF)
option(${PROJECT_NAME}_EXAMPLES "Build the examples" ON)
option(${PROJECT_NAME}_DOCS "Build the API documentation" OFF)
option(${PROJECT_NAME}_QT6 "Build against Qt 6" OFF)

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 99)
set(${PROJECT_NAME}_VERSION_PATCH 50)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH})
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

find_path(BOOST_OPTIONAL_DIR NAMES boost/optional.hpp)
if(BOOST_OPTIONAL_DIR)
  message(STATUS "Found boost/optional.hpp in ${BOOST_OPTIONAL_DIR}")
  include_directories(${BOOST_OPTIONAL_DIR})
  add_definitions(-DBOOST_OPTIONAL_FOUND) # I'd really prefer using configure_file but this is more compatible with qmake
endif()

if(${PROJECT_NAME}_QT6)
  find_package(Qt6 6.0.0 CONFIG REQUIRED COMPONENTS Core Network)
  set(QT_MAJOR_VERSION 6)
  list(APPEND QT_LIBRARIES Qt6::Core Qt6::Network)
  set(${PROJECT_NAME}_LIBRARY_QTID "-qt6")
  set(libkode_QT6 ON)
else()
  # Qt 5.9 required for QNetworkRequest::NoLessSafeRedirectPolicy
  set(QT_MIN_VERSION "5.9.0")
  find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Network)
  set(QT_MAJOR_VERSION 5)
  list(APPEND QT_LIBRARIES Qt5::Core Qt5::Network)
  set(${PROJECT_NAME}_LIBRARY_QTID "")
endif()

# setup default install locations
include(InstallLocation)
include(KDQtInstallPaths) #to set QT_INSTALL_FOO variables

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC ON)

# Default to hidden visibility for symbols
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_definitions(-DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQURL_NO_CAST_FROM_STRING)
add_definitions(-DUSE_EXCEPTIONS -DQT_FATAL_ASSERT)
if(MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-Wsuggest-override HAVE_GXX_SUGGEST_OVERRIDE)
  if(HAVE_GXX_SUGGEST_OVERRIDE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override")
  endif()
endif()

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

  if(USE_DEFAULT_INSTALL_LOCATION)
    if(UNIX)
      set(CMAKE_INSTALL_PREFIX "/usr/local/KDAB/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
    elseif(WIN32)
      set(CMAKE_INSTALL_PREFIX "C:\\KDAB\\${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}")
    endif()
  endif()

  message(STATUS "Building ${PROJECT_NAME} ${${PROJECT_NAME}_VERSION} in ${CMAKE_BUILD_TYPE} mode. Installing to ${CMAKE_INSTALL_PREFIX}")

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

  install(FILES README.txt README-commercial.txt kdsoap.pri kdwsdl2cpp.pri DESTINATION ${INSTALL_DOC_DIR})
  install(DIRECTORY LICENSES DESTINATION ${INSTALL_DOC_DIR})

  # Generate library version files
  include(ECMSetupVersion)
  ecm_setup_version(
    ${${PROJECT_NAME}_VERSION}
    VARIABLE_PREFIX KDSOAP
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdsoap_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapConfigVersion.cmake"
    SOVERSION ${${PROJECT_NAME}_SOVERSION}
    COMPATIBILITY AnyNewerVersion
  )
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kdsoap_version.h" DESTINATION ${INSTALL_INCLUDE_DIR}/KDSoapClient)

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDSoapConfig-buildtree.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapConfig.cmake"
    @ONLY
  )
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDSoapMacros.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapMacros.cmake"
    @ONLY
  )
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDSoapConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/install/KDSoapConfig.cmake"
    INSTALL_DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/KDSoap"
    PATH_VARS INSTALL_INCLUDE_DIR
  )

  install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/install/KDSoapConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapConfigVersion.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapMacros.cmake"
    DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/KDSoap"
  )
  install(EXPORT KDSoapTargets NAMESPACE KDSoap::
    DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/KDSoap"
  )

  # Generate .pri file for qmake users
  include(ECMGeneratePriFile)
  set(PROJECT_VERSION_STRING ${${PROJECT_NAME}_VERSION})
  ecm_generate_pri_file(BASE_NAME KDSoapClient
                        LIB_NAME kdsoap
                        FILENAME_VAR pri_client_filename
  )
  ecm_generate_pri_file(BASE_NAME KDSoapServer
                        LIB_NAME kdsoap-server
                        FILENAME_VAR pri_server_filename
  )
  install(FILES ${pri_client_filename} ${pri_server_filename} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
endif()

add_subdirectory(src)
add_subdirectory(kdwsdl2cpp)
if(${PROJECT_NAME}_IS_ROOT_PROJECT)

  export(TARGETS kdsoap kdsoap-server kdwsdl2cpp NAMESPACE KDSoap::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapTargets.cmake"
  )

  add_subdirectory(features)
  if(${PROJECT_NAME}_TESTS)
    enable_testing()
    add_subdirectory(testtools)
    add_subdirectory(unittests)
  endif()
  if(${PROJECT_NAME}_EXAMPLES)
    add_subdirectory(examples)
  endif()

  if(${PROJECT_NAME}_DOCS)
    add_subdirectory(docs) # needs to go last, in case there are build source files
  else()
    add_custom_target(docs
      COMMAND ${CMAKE_COMMAND} -E echo "Sorry, there is no docs target since KDSoap_DOCS=OFF."
      "Re-run cmake with the -DKSoap_DOCS=True option if you want to generate the documentation.")
  endif()

  # Add uninstall target
  include(ECMUninstallTarget)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
