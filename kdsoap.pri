# Not used by KDSoap itself. This is for use in other projects.
# Copy the file there, but backport any changes here.

:  # copy from environment:
  isEmpty( KDSOAPDIR ):KDSOAPDIR="$$(KDSOAPDIR)"
  !isEmpty( KDSOAPDIR ) {
    unix:isEmpty(QMAKE_EXTENSION_SHLIB):QMAKE_EXTENSION_SHLIB=so
    unix:!exists( $$KDSOAPDIR/lib/libkdsoap.$$QMAKE_EXTENSION_SHLIB ):error( "Cannot find libkdsoap.$$QMAKE_EXTENSION_SHLIB in $KDSOAPDIR/lib" )
    #win32:!exists( $$KDSOAPDIR/lib/kdsoap.lib ):error( "Cannot find kdsoap.lib in $KDSOAPDIR/lib" )
    unix:!exists( $$KDSOAPDIR/src/KDSoapClientInterface.h ):error( "Cannot find KDSoapClientInterface.h in $KDSOAPDIR/src" )

    LIBS += -L$$KDSOAPDIR/lib
    win32* {
      CONFIG(debug, debug|release) {
        LIBS += -lkdsoapd
      } else {
        LIBS += -lkdsoap
      }
    } else {
      !isEmpty(QMAKE_LFLAGS_RPATH):LIBS += $$QMAKE_LFLAGS_RPATH$$KDSOAPDIR/lib
      LIBS += -lkdsoap
    }

    INCLUDEPATH += $$KDSOAPDIR/include $$KDSOAPDIR/src
    DEPENDPATH += $$KDSOAPDIR/include $$KDSOAPDIR/src

    CONFIG += have_kdsoap
    DEFINES += HAVE_KDSOAP

    include($$KDSOAPDIR/kdwsdl2cpp.pri)

  } else:equals( builddir, $$top_builddir ) {
    message( "WARNING: kdsoap not found. Please set KDSOAPDIR either as an environment variable or on the qmake command line if you want kdsoap support")
  }
