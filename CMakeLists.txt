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
# (See INSTALL.txt for more information)
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

cmake_minimum_required(VERSION 3.3)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kdwsdl2cpp/libkode/common")
  message(FATAL_ERROR "Please do git submodule update --init --recursive")
endif()

# Just to fix warnings with --warn-uninitialized
if(NOT DEFINED USE_DEFAULT_INSTALL_LOCATION)
  set(USE_DEFAULT_INSTALL_LOCATION FALSE)
endif()
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "")
endif()

# Allow using a non-KDAB install location
set(KDAB_INSTALL True)
if((NOT DEFINED ${USE_DEFAULT_INSTALL_LOCATION}) OR (NOT ${USE_DEFAULT_INSTALL_LOCATION}))
  if(NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set(KDAB_INSTALL False)
  endif()
  set(USE_DEFAULT_INSTALL_LOCATION ${KDAB_INSTALL} CACHE INTERNAL "Install to default KDAB Location" FORCE)
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  project(KDSoap LANGUAGES CXX)
else()
  project(KDSoap
          DESCRIPTION "A Qt-based client-side and server-side SOAP component"
          HOMEPAGE_URL "https://github.com/KDAB/KDSoap"
          LANGUAGES CXX)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ECM/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDAB/modules")

include(FeatureSummary)

option(${PROJECT_NAME}_STATIC "Build statically" OFF)
option(${PROJECT_NAME}_TESTS "Build the tests" OFF)
option(${PROJECT_NAME}_EXAMPLES "Build the examples" ON)
option(${PROJECT_NAME}_DOCS "Build the API documentation" OFF)
option(${PROJECT_NAME}_QT6 "Build against Qt 6" OFF)

set(${PROJECT_NAME}_VERSION_MAJOR 2)
set(${PROJECT_NAME}_VERSION_MINOR 1)
set(${PROJECT_NAME}_VERSION_PATCH 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH})
set(PROJECT_VERSION ${${PROJECT_NAME}_VERSION}) #needed for ECM
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

find_path(BOOST_OPTIONAL_DIR NAMES boost/optional.hpp)
if(BOOST_OPTIONAL_DIR)
  message(STATUS "Found boost/optional.hpp in ${BOOST_OPTIONAL_DIR}")
  include_directories(${BOOST_OPTIONAL_DIR})
  add_definitions(-DBOOST_OPTIONAL_FOUND) # I'd really prefer using configure_file but this is more compatible with qmake
endif()

# Set a default build type if none was specified
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git" OR ${PROJECT_NAME}_DEVELOPER_MODE)
  set(default_build_type "Debug")
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to ${default_build_type} as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(${PROJECT_NAME}_QT6)
  set(Qt_VERSION_MAJOR 6)
  set(QT_MIN_VERSION "6.0.0")
  find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Network)
  list(APPEND QT_LIBRARIES Qt6::Core Qt6::Network)
  set(${PROJECT_NAME}_LIBRARY_QTID "-qt6")
  set(libkode_QT6 ON)
else()
  set(Qt_VERSION_MAJOR 5)
  # Qt 5.9 required for QNetworkRequest::NoLessSafeRedirectPolicy
  set(QT_MIN_VERSION "5.9.0")
  find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Network)
  list(APPEND QT_LIBRARIES Qt5::Core Qt5::Network)
  set(${PROJECT_NAME}_LIBRARY_QTID "")
endif()

# setup default install locations
include(InstallLocation)
include(KDQtInstallPaths) #to set QT_INSTALL_FOO variables

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to hidden visibility for symbols
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_definitions(-DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQURL_NO_CAST_FROM_STRING)
add_definitions(-DUSE_EXCEPTIONS -DQT_FATAL_ASSERT -DQT_NO_FOREACH)
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

  install(FILES README.txt README-commercial.txt DESTINATION ${INSTALL_DOC_DIR})
  if(NOT ${PROJECT_NAME}_QT6)
    install(FILES kdsoap.pri kdwsdl2cpp.pri DESTINATION ${INSTALL_DOC_DIR})
  endif()
  install(DIRECTORY LICENSES DESTINATION ${INSTALL_DOC_DIR})

  # Generate library version files
  include(ECMSetupVersion)
  ecm_setup_version(
    ${${PROJECT_NAME}_VERSION}
    VARIABLE_PREFIX KDSOAP
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdsoap_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}ConfigVersion.cmake"
    SOVERSION ${${PROJECT_NAME}_SOVERSION}
    COMPATIBILITY AnyNewerVersion
  )
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kdsoap_version.h" DESTINATION ${INSTALL_INCLUDE_DIR}/KDSoapClient)

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDSoapConfig-buildtree.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}Config.cmake"
    @ONLY
  )
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDSoapMacros.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapMacros.cmake"
    @ONLY
  )
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDSoapConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/install/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}Config.cmake"
    INSTALL_DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}"
    PATH_VARS INSTALL_INCLUDE_DIR
  )

  install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/install/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}Config.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}ConfigVersion.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/KDSoap/KDSoapMacros.cmake"
    DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}"
  )
  install(EXPORT KDSoapTargets NAMESPACE KDSoap::
    DESTINATION "${INSTALL_LIBRARY_DIR}/cmake/KDSoap${${PROJECT_NAME}_LIBRARY_QTID}"
  )

  # Generate .pri file for qmake users
  #TODO: ECM does not support Qt6 yet
  if(Qt_VERSION_MAJOR EQUAL 5 AND
     CMAKE_VERSION VERSION_GREATER "3.11.99" AND NOT CMAKE_CONFIGURATION_TYPES) # Not working with VS generator or older cmake versions
    include(ECMGeneratePriFile)
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
