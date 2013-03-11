# Some default installation locations. These should be global, with any project
# specific locations added to the end. These paths are all relative to the
# install prefix.
#
# These paths attempt to adhere to the FHS, and are similar to those provided
# by autotools and used in many Linux distributions.

# Use CMAKE_INSTALL_LIBDIR for library install locations
include(GNUInstallDirs)

if(NOT INSTALL_RUNTIME_DIR)
  set(INSTALL_RUNTIME_DIR "bin")
endif()
if(NOT INSTALL_LIBRARY_DIR)
  set(INSTALL_LIBRARY_DIR  ${CMAKE_INSTALL_LIBDIR})
endif()
if(NOT INSTALL_ARCHIVE_DIR)
  set(INSTALL_ARCHIVE_DIR  ${CMAKE_INSTALL_LIBDIR})
endif()
if(NOT INSTALL_INCLUDE_DIR)
  set(INSTALL_INCLUDE_DIR "include")
endif()
