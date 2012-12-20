/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "license.h"

using namespace KODE;

class License::Private
{
  public:
    Private()
      : mType(License::NoLicense), mQtException( false )
    {
    }

    Type mType;
    bool mQtException;
};

License::License()
  : d( new Private )
{
}

License::License( const License &other )
  : d( new Private )
{
  *d = *other.d;
}

License::License( Type type )
  : d( new Private )
{
  d->mType = type;
}

License::~License()
{
  delete d;
}

License& License::operator=( const License &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void License::setQtException( bool v )
{
  d->mQtException = v;
}

QString License::text() const
{
  QString txt;

  switch ( d->mType ) {
    case GPL:
      txt +=
            QLatin1String("This program is free software; you can redistribute it and/or modify\n"
            "it under the terms of the GNU General Public License as published by\n"
            "the Free Software Foundation; either version 2 of the License, or\n"
            "(at your option) any later version.\n"
            "\n"
            "This program is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
            "GNU General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU General Public License\n"
            "along with this program; if not, write to the Free Software\n"
            "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,\n" "USA.\n");

      if ( d->mQtException ) {
        txt += QLatin1Char('\n');
        txt +=
            QLatin1String("As a special exception, permission is given to link this program\n"
            "with any edition of Qt, and distribute the resulting executable,\n"
            "without including the source code for Qt in the source distribution.\n");
      }
      break;
    case LGPL:
      txt += QLatin1String(
            "This library is free software; you can redistribute it and/or\n"
            "modify it under the terms of the GNU Library General Public\n"
            "License as published by the Free Software Foundation; either\n"
            "version 2 of the License, or (at your option) any later version.\n"
            "\n"
            "This library is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
            "Library General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU Library General Public License\n"
            "along with this library; see the file COPYING.LIB.  If not, write to\n"
            "the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
            "Boston, MA 02110-1301, USA.\n");
      break;
    case BSD:
      txt += QLatin1String(
            "Permission is hereby granted, free of charge, to any person obtaining\n"
            "a copy of this software and associated documentation files (the\n"
            "\"Software\"), to deal in the Software without restriction, including\n"
            "without limitation the rights to use, copy, modify, merge, publish,\n"
            "distribute, sublicense, and/or sell copies of the Software, and to\n"
            "permit persons to whom the Software is furnished to do so, subject to\n"
            "the following conditions:\n"
            "\n"
            "The above copyright notice and this permission notice shall be\n"
            "included in all copies or substantial portions of the Software.\n"
            "\n"
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
            "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
            "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT\n"
            "IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR\n"
            "OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,\n"
            "ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n"
            "OTHER DEALINGS IN THE SOFTWARE.");
      break;
    default:
      break;
  }

  return txt;
}
