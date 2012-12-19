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

#include <QtCore/QStringList>

#include "file.h"
#include <QDebug>

using namespace KODE;

class File::Private
{
  public:
    Private()
      : mProject()
    {
    }

    QString mHeaderFilename;
    QString mImplFilename;
    QString mNameSpace;
    QString mProject;
    QStringList mCopyrightStrings;
    License mLicense;
    QStringList mIncludes;
    Class::List mClasses;
    Variable::List mFileVariables;
    Function::List mFileFunctions;
    Enum::List mFileEnums;
    QStringList mExternCDeclarations;
    Code mFileCode;
};

File::File()
  : d( new Private )
{
}

File::File( const File &other )
  : d( new Private )
{
  *d = *other.d;
}

File::~File()
{
  delete d;
}

File& File::operator=( const File &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void File::setFilename( const QString& filename )
{
  d->mImplFilename = filename + QLatin1String(".cpp");
  d->mHeaderFilename = filename + QLatin1String(".h");
}

void File::setImplementationFilename( const QString &filename )
{
  d->mImplFilename = filename;
}

void File::setHeaderFilename( const QString &filename )
{
  d->mHeaderFilename = filename;
}

QString File::filenameHeader() const
{
    if ( !d->mHeaderFilename.isEmpty() )
      return d->mHeaderFilename;

    if ( !d->mClasses.isEmpty() ) {
      QString className = d->mClasses[ 0 ].name();
      return className.toLower() + QLatin1String(".h");
    }

    return QString();
}

QString File::filenameImplementation() const
{
    if ( !d->mImplFilename.isEmpty() )
      return d->mImplFilename;

    if ( !d->mClasses.isEmpty() ) {
      QString className = d->mClasses[ 0 ].name();
      return className.toLower() + QLatin1String(".cpp");
    }

    return QString();
}

void File::setNameSpace( const QString &nameSpace )
{
  d->mNameSpace = nameSpace;
}

QString File::nameSpace() const
{
  return d->mNameSpace;
}

void File::setProject( const QString &project )
{
  if ( project.isEmpty() )
    return;

  d->mProject = project;
}

QString File::project() const
{
  return d->mProject;
}

void File::addCopyright( int year, const QString &name, const QString &email )
{
  QString str = QLatin1String("Copyright (c) ") + QString::number( year ) + QLatin1Char(' ') + name + QLatin1String(" <")
                + email + QLatin1Char('>');

  d->mCopyrightStrings.append( str );
}

QStringList File::copyrightStrings() const
{
  return d->mCopyrightStrings;
}

void File::setLicense( const License &license )
{
  d->mLicense = license;
}

License File::license() const
{
  return d->mLicense;
}

void File::addInclude( const QString &_include )
{
  QString include = _include;
  if ( !include.endsWith( QLatin1String(".h") ) )
    include.append( QLatin1String(".h") );

  if ( !d->mIncludes.contains( include ) )
    d->mIncludes.append( include );
}

QStringList File::includes() const
{
  return d->mIncludes;
}

void File::insertClass( const Class &newClass )
{
  Q_ASSERT(!newClass.name().isEmpty());
  Class::List::Iterator it;
  for ( it = d->mClasses.begin(); it != d->mClasses.end(); ++it ) {
    if ( (*it).qualifiedName() == newClass.qualifiedName() ) {
      // This happens often, probably due to usage of shared types
      //qDebug() << "WARNING: Already having class" << newClass.qualifiedName() << "in file" << filenameHeader() << filenameImplementation();
      it = d->mClasses.erase( it );
      d->mClasses.insert( it, newClass );
      return;
    }
  }

  d->mClasses.append( newClass );
}

Class::List File::classes() const
{
  return d->mClasses;
}

bool File::hasClass( const QString &name )
{
  Class::List::ConstIterator it;
  for ( it = d->mClasses.constBegin(); it != d->mClasses.constEnd(); ++it ) {
    if ( (*it).name() == name )
      break;
  }

  return it != d->mClasses.constEnd();
}

Class File::findClass( const QString &name )
{
  Class::List::ConstIterator it;
  for ( it = d->mClasses.constBegin(); it != d->mClasses.constEnd(); ++it ) {
    if ( (*it).name() == name )
      return *it;
  }

  return Class();
}

void File::addFileVariable( const Variable &variable )
{
  d->mFileVariables.append( variable );
}

Variable::List File::fileVariables() const
{
  return d->mFileVariables;
}

void File::addFileFunction( const Function &function )
{
  d->mFileFunctions.append( function );
}

Function::List File::fileFunctions() const
{
  return d->mFileFunctions;
}

void File::addFileEnum( const Enum &enumValue )
{
  d->mFileEnums.append( enumValue );
}

Enum::List File::fileEnums() const
{
  return d->mFileEnums;
}

void File::addExternCDeclaration( const QString &externalCDeclaration )
{
  d->mExternCDeclarations.append( externalCDeclaration );
}

QStringList File::externCDeclarations() const
{
  return d->mExternCDeclarations;
}

void File::addFileCode( const Code &code )
{
  d->mFileCode = code;
}

Code File::fileCode() const
{
  return d->mFileCode;
}

void File::clearClasses()
{
  d->mClasses.clear();
}

void File::clearFileFunctions()
{
  d->mFileFunctions.clear();
}

void File::clearFileVariables()
{
  d->mFileVariables.clear();
}

void File::clearCode()
{
  clearClasses();
  clearFileFunctions();
  clearFileVariables();
}
