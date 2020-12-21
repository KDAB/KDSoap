## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

# Not used by KDSoap itself. This is for use in other projects.
# Copy the file there, but backport any changes here.

  # copy from environment:
  isEmpty( KDSOAPDIR ):KDSOAPDIR="$$(KDSOAPDIR)"
  !isEmpty( KDSOAPDIR ) {
    unix {
      static:!exists( $$KDSOAPDIR/lib/libkdsoap.a ) {
        error( "Cannot find libkdsoap.a in $KDSOAPDIR/lib" )
      } else {
        isEmpty(QMAKE_EXTENSION_SHLIB) {
          macx:QMAKE_EXTENSION_SHLIB=dylib
          else:QMAKE_EXTENSION_SHLIB=so
        }
        !exists( $$KDSOAPDIR/lib/libkdsoap.$$QMAKE_EXTENSION_SHLIB ):!exists( $$KDSOAPDIR/lib/libkdsoap.a ) {
          error( "Cannot find libkdsoap.$$QMAKE_EXTENSION_SHLIB or libkdsoap.a in $$KDSOAPDIR/lib" )
        }
      }
      !exists( $$KDSOAPDIR/include/KDSoapClient/KDSoapClientInterface.h ):error( "Cannot find KDSoapClientInterface.h in $KDSOAPDIR/include/KDSoapClient" )
    }
    #win32:!exists( $$KDSOAPDIR/lib/kdsoap.lib ):error( "Cannot find kdsoap.lib in $KDSOAPDIR/lib" )

    LIBS += -L$$KDSOAPDIR/lib
    KDSOAPSERVERLIB = kdsoap-server
    win32* {
      CONFIG(debug, debug|release) {
        LIBS += -lkdsoapd
        KDSOAPSERVERLIB = kdsoap-serverd
      } else {
        LIBS += -lkdsoap
      }
    } else {
      !isEmpty(QMAKE_LFLAGS_RPATH):LIBS += $$QMAKE_LFLAGS_RPATH$$KDSOAPDIR/lib
      LIBS += -lkdsoap
    }
    QT += network

    INCLUDEPATH += $$KDSOAPDIR/include $$KDSOAPDIR/include/KDSoapClient $$KDSOAPDIR/include/KDSoapServer
    DEPENDPATH += $$KDSOAPDIR/include $$KDSOAPDIR/include/KDSoapClient $$KDSOAPDIR/include/KDSoapServer

    CONFIG += have_kdsoap
    DEFINES += HAVE_KDSOAP

    exists($$KDSOAPDIR/kdwsdl2cpp.pri) {
       include($$KDSOAPDIR/kdwsdl2cpp.pri)
    } else:exists($$KDSOAPDIR/share/doc/KDSoap/kdwsdl2cpp.pri) {
       include($$KDSOAPDIR/share/doc/KDSoap/kdwsdl2cpp.pri)
    } else {
        message("WARNING: kdwsdl2cpp.pri not found in KDSOAPDIR=$$KDSOAPDIR")
    }

  } else:equals( builddir, $$top_builddir ) {
    message( "WARNING: kdsoap not found. Please set KDSOAPDIR either as an environment variable or on the qmake command line if you want kdsoap support")
  }
