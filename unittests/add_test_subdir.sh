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

# QMake

cat > $subdir/${subdir}.pro <<EOF
include( \$\${TOP_SOURCE_DIR}/unittests/unittests.pri )
TARGET = test_${subdir}
QT += network xml
SOURCES = $cppfiles
test.target = test
test.commands = ./\$(TARGET)
test.depends = \$(TARGET)
QMAKE_EXTRA_TARGETS += test

KDWSDL = ${wsdlfiles}

OTHER_FILES = \$\$KDWSDL
LIBS        += -L\$\${TOP_BUILD_DIR}/lib
EOF

git add $subdir/${subdir}.pro
perl -pi -e '$_ .= "  '$subdir' \\\n" if (/vidyo/)' unittests.pro

