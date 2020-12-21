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

*-g++* {
  NORMAL_CFLAGS = -Wno-long-long
  win32-g++ {
    NORMAL_CFLAGS += -U__STRICT_ANSI__
  }

  NORMAL_CXXFLAGS = \
	-Wnon-virtual-dtor -Wundef -Wcast-align \
	-Wchar-subscripts -Wpointer-arith \
	-Wwrite-strings -Wformat -Wformat-security \
        -Wmissing-format-attribute -Woverloaded-virtual -std=c++11

  # -Wconversion gives too many warnings from Qt-4.4.3 with gcc-4.3.2 (was fine with gcc-4.2.4), so removing it
  # -Wpacked gives warnings with Qt 4.8, removed

  NORMAL_CFLAGS += -pedantic

  CONFIG(debug, debug|release) {
    NORMAL_CXXFLAGS += -O0 -g3
    NORMAL_CXXFLAGS -= -g
  }
  USABLE_CXXFLAGS = -Wold-style-cast
  HARD_CXXFLAGS = -Weffc++ -Wshadow
  PITA_CXXFLAGS = -Wunreachable-code

  QMAKE_CFLAGS   += $$NORMAL_CFLAGS
  QMAKE_CXXFLAGS += $$NORMAL_CFLAGS $$NORMAL_CXXFLAGS

  kdab:QMAKE_CFLAGS_WARN_ON   += $$NORMAL_CFLAGS -Werror
  kdab:QMAKE_CXXFLAGS_WARN_ON += $$NORMAL_CFLAGS $$NORMAL_CXXFLAGS -Werror

  #QMAKE_CXXFLAGS_WARN_ON += $$USABLE_CXXFLAGS
  #QMAKE_CXXFLAGS_WARN_ON += $$HARD_CXXFLAGS # headers must compile with this, code doesn't need to; needs patched Qt
  #QMAKE_CXXFLAGS_WARN_ON += $$PITA_CXXFLAGS # header would be nice, but it's probably pointless, due to noise from Qt and libstdc++

}
