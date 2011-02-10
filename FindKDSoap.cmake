# - Find KDSoap
# This module finds if KDSoap is installed.
#
# KDSoap_FOUND		- Set to TRUE if KDSoap was found.
# KDSoap_LIBRARIES	- Path to KDSoap libraries.
# KDSoap_INCLUDE_DIR	- Path to the KDSoap include directory.
#
# This file was generated automatically.
# Please edit generate-configure.sh rather than this file.

include(FindPackageHandleStandardArgs)

find_library(KDSoap_LIBRARIES 
	NAMES KDSoap kdsoap 
	PATH_SUFFIXES bin)
find_path(KDSoap_INCLUDE_DIR 
	NAMES KDSoap KDSoapGlobal)

mark_as_advanced(KDSoap_LIBRARIES KDSoap_INCLUDE_DIR)

find_package_handle_standard_args(KDSoap DEFAULT_MSG KDSoap_LIBRARIES KDSoap_INCLUDE_DIR)
