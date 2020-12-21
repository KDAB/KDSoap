## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2009-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

CONFIG += qt warn_on

exists( g++.pri ):include( g++.pri )

DEFINES += USE_EXCEPTIONS QT_FATAL_ASSERT

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QT_NO_CAST_FROM_BYTEARRAY

solaris-cc:DEFINES += SUN7

# for C++11 support
mac: QMAKE_CXXFLAGS += -stdlib=libc++

win32-msvc*:DEFINES += _SCL_SECURE_NO_WARNINGS

win32-msvc*:QMAKE_CXXFLAGS += /GR /EHsc /wd4251
unix:!macx:QMAKE_LFLAGS += -Wl,-no-undefined
macx:QMAKE_SONAME_PREFIX = @rpath

CONFIG += depend_includepath

QT += network

contains(TEMPLATE, lib) {
  DESTDIR = $${TOP_BUILD_DIR}/lib
}

contains(TEMPLATE, app) {
  DESTDIR = $${TOP_BUILD_DIR}/bin
}

staticlib {
} else {
  contains(TEMPLATE, lib) {
    win32 {
      DLLDESTDIR = $${TOP_BUILD_DIR}/bin
      CONFIG += dll
#skip_target_version_ext was introduced in Qt5.3 so we can't use it for Qt4.8 builds
#so fallback to setting the empty VERSION trick.
      VERSION=
#      CONFIG += skip_target_version_ext
    }
  }
}

unix:!symbian {
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  UI_DIR = .ui
} else {
  MOC_DIR = _moc
  OBJECTS_DIR = _obj
  UI_DIR = _ui
}
