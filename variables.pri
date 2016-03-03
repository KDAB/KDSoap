CONFIG += qt warn_on

exists( g++.pri ):include( g++.pri )

DEFINES += USE_EXCEPTIONS QT_FATAL_ASSERT

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QT_NO_CAST_FROM_BYTEARRAY

solaris-cc:DEFINES += SUN7

win32-msvc*:DEFINES += _SCL_SECURE_NO_WARNINGS

win32-msvc*:QMAKE_CXXFLAGS += /GR /EHsc /wd4251

unix:!macx:QMAKE_LFLAGS += -Wl,-no-undefined

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
