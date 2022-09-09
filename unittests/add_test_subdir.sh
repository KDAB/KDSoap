#!/bin/sh
#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2017-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

# Generate buildsystem files for a new subdir

unset CDPATH

subdir=$1
test -z "$subdir" && exit 1
test -d "$subdir" || exit 2

wsdlfiles=`cd $subdir ; ls -1 *.wsdl`
cppfiles=`cd $subdir ; ls -1 *.cpp`

# CMake

cat > $subdir/CMakeLists.txt <<EOF
set(${subdir}_SRCS $cppfiles)
set(WSDL_FILES ${wsdlfiles})
add_unittest(\${${subdir}_SRCS})
EOF

git add $subdir/CMakeLists.txt
echo "add_subdirectory($subdir)" >> CMakeLists.txt
