CONFIG += qt warn_on

exists( g++.pri ):include( g++.pri )

DEFINES += USE_EXCEPTIONS QT_FATAL_ASSERT

DEFINES += QT_NO_STL QT_NO_CAST_TO_ASCII QBA_NO_CAST_TO_VOID QBA_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

solaris-cc:DEFINES += SUN7

win32-msvc*:QMAKE_CXXFLAGS += /GR /EHsc /wd4251

unix:!macx:QMAKE_LFLAGS += -Wl,-no-undefined

CONFIG += depend_includepath

QT += network

contains(TEMPLATE, lib) {
  DESTDIR = $$PWD/lib
}

contains(TEMPLATE, app) {
  DESTDIR = $$PWD/bin
}

staticlib {
} else {
  contains(TEMPLATE, lib) {
    win32 {
      DLLDESTDIR = $$PWD/bin
      CONFIG += dll
    }
  }
}

unix {
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  UI_DIR = .ui
} else {
  MOC_DIR = _moc
  OBJECTS_DIR = _obj
  UI_DIR = _ui
}
