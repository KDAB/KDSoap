/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <libkode/file.h>
#include <libkode/printer.h>

#include "settings.h"

#include "creator.h"

using namespace KWSDL;

/**
 * This method sorts a list of classes in a way that the base class
 * of a class always appears before the class itself.
 */
static KODE::Class::List sortByBaseClass( const KODE::Class::List &classes )
{
  KODE::Class::List allClasses( classes );
  KODE::Class::List retval;

  QStringList classNames;

  // copy all classes without a base class
  KODE::Class::List::Iterator it;
  for ( it = allClasses.begin(); it != allClasses.end(); ++it ) {
    if ( (*it).baseClasses().isEmpty() || (*it).baseClasses().first().name().startsWith( "Q" ) ) {
      retval.append( *it );
      classNames.append( (*it).name() );

      it = allClasses.erase( it );
      it--;
    }
  }

  while ( allClasses.count() > 0 ) {
    // copy all classes which have a class from retval
    // as base class
    for ( it = allClasses.begin(); it != allClasses.end(); ++it ) {
      const QString baseClassName = (*it).baseClasses().first().name();

      if ( classNames.contains( baseClassName ) ) {
        retval.append( *it );
        classNames.append( (*it).name() );

        it = allClasses.erase( it );
        it--;
      }
    }
  }

  return retval;
}

Creator::Creator()
{
}

void Creator::create( const KODE::Class::List &list )
{
  KODE::Printer printer;
  printer.setOutputDirectory( Settings::self()->outputDirectory() );

  // Set generated header details.
  printer.setCreationWarning( true );
  printer.setGenerator( QLatin1String( "kwsdl_compiler" ) );
  printer.setSourceFile( Settings::self()->wsdlFileName() );

  const KODE::Class::List classes = sortByBaseClass( list );

  KODE::File file;

  file.setFilename( Settings::self()->outputFileName() );

  KODE::Class::List::ConstIterator it;
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
    file.insertClass( *it );
  }

  printer.printHeader( file );
  printer.printImplementation( file );
}
