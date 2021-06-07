## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2017-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##
#!/bin/sh

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

