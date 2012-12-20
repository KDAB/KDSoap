/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2010 David Faure <dfaure@kdab.com>

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

#include "class.h"

#include <QDebug>

using namespace KODE;

class Class::Private
{
  public:
    Private()
      : mDPointer(),
        mUseSharedData( false ),
        mCanBeCopied( false )
    {
    }

    QString mName;
    QString mNameSpace;
    QString mExportDeclaration;
    QString mDPointer;
    bool mUseSharedData;
    bool mCanBeCopied;
    Function::List mFunctions;
    MemberVariable::List mMemberVariables;
    QStringList mIncludes;
    QStringList mForwardDeclarations;
    QStringList mHeaderIncludes;
    Class::List mBaseClasses;
    Typedef::List mTypedefs;
    Enum::List mEnums;
    QString mDocs;
    Class::List mNestedClasses;
    QString mParentClassName;
    QStringList mDeclMacros;
};

Class::Class()
  : d( new Private )
{
}

Class::Class( const Class &other )
  : d( new Private )
{
  *d = *other.d;
}

Class::Class( const QString &name, const QString &nameSpace )
  : d( new Private )
{
  Q_ASSERT(!name.isEmpty());
  d->mName = name;
  d->mNameSpace = nameSpace;
}

Class::~Class()
{
  delete d;
}

Class& Class::operator=( const Class &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void Class::setName( const QString &name )
{
  Q_ASSERT(!name.isEmpty());
  d->mName = name;
}

QString Class::name() const
{
  return d->mName;
}

void Class::setNameSpace( const QString &nameSpace )
{
  d->mNameSpace = nameSpace;
}

QString Class::nameSpace() const
{
  return d->mNameSpace;
}

QString Class::qualifiedName() const
{
  if (d->mNameSpace.isEmpty())
      return d->mName;
  return d->mNameSpace + QLatin1String("::") + d->mName;
}

void Class::setExportDeclaration( const QString &name )
{
  addHeaderInclude( name.toLower() + QLatin1String("_export.h") );
  if ( name.contains( QLatin1String("/") ) ) {
    d->mExportDeclaration = name.split( QLatin1String("/") ).value( 1 );
  } else {
    d->mExportDeclaration = name;
  }
}

QString Class::exportDeclaration() const
{
  return d->mExportDeclaration;
}

void Class::setUseDPointer( bool useDPointer, const QString& dPointer )
{
  d->mDPointer = useDPointer ? dPointer : QString();
}

bool Class::useDPointer() const
{
  return !d->mDPointer.isEmpty();
}

void Class::setUseSharedData( bool b, const QString& dPointer )
{
  d->mUseSharedData = b;
  if ( b ) {
    setUseDPointer( true, dPointer );
    d->mCanBeCopied = true;
  }
}

bool Class::useSharedData() const
{
  return d->mUseSharedData;
}

void Class::setCanBeCopied( bool b )
{
  d->mCanBeCopied = b;
}

bool Class::canBeCopied() const
{
  return d->mCanBeCopied;
}

void Class::addInclude( const QString &include,
                        const QString &forwardDeclaration )
{
  if ( !include.isEmpty() && !d->mIncludes.contains( include ) )
      d->mIncludes.append( include );

  if ( !forwardDeclaration.isEmpty() &&
       !d->mForwardDeclarations.contains( forwardDeclaration ) )
    d->mForwardDeclarations.append( forwardDeclaration );
}

void Class::addIncludes( const QStringList &files,
                         const QStringList &forwardDeclarations )
{
  for ( int i = 0; i < files.count(); ++i ) {
    if ( !d->mIncludes.contains( files[ i ] ) )
      if ( !files[ i ].isEmpty() )
        d->mIncludes.append( files[ i ] );
  }

  for ( int i = 0; i < forwardDeclarations.count(); ++i ) {
    if ( !d->mForwardDeclarations.contains( forwardDeclarations[ i ] ) )
      d->mForwardDeclarations.append( forwardDeclarations[ i ] );
  }
}

QStringList Class::includes() const
{
  return d->mIncludes;
}

QStringList Class::forwardDeclarations() const
{
  return d->mForwardDeclarations;
}

void Class::addHeaderInclude( const QString &include )
{
  if ( include.isEmpty() )
    return;

  if ( !d->mHeaderIncludes.contains( include ) )
    d->mHeaderIncludes.append( include );
}

void Class::addHeaderIncludes( const QStringList &includes )
{
  QStringList::ConstIterator it;
  for ( it = includes.constBegin(); it != includes.constEnd(); ++it )
    addHeaderInclude( *it );
}

QStringList Class::headerIncludes() const
{
  return d->mHeaderIncludes;
}

void Class::addBaseClass( const Class &c )
{
  d->mBaseClasses.append( c );
}

Class::List Class::baseClasses() const
{
  return d->mBaseClasses;
}

void Class::addFunction( const Function &function )
{
  d->mFunctions.append( function );
}

Function::List Class::functions() const
{
  return d->mFunctions;
}

void Class::addMemberVariable( const MemberVariable &v )
{
  d->mMemberVariables.append( v );
}

MemberVariable::List Class::memberVariables() const
{
  return d->mMemberVariables;
}

void Class::addTypedef( const Typedef &typeDefinition )
{
  d->mTypedefs.append( typeDefinition );
}

Typedef::List Class::typedefs() const
{
  return d->mTypedefs;
}

void Class::addEnum( const Enum &enumValue )
{
  d->mEnums.append( enumValue );
}

Enum::List Class::enums() const
{
  return d->mEnums;
}

bool Class::hasEnum( const QString &name ) const
{
  foreach( Enum e, d->mEnums ) {
    if ( e.name() == name ) return true;
  }
  return false;
}

bool Class::isValid() const
{
  return !d->mName.isEmpty();
}

bool Class::hasFunction( const QString &functionName ) const
{
  Function::List::ConstIterator it;
  for ( it = d->mFunctions.constBegin(); it != d->mFunctions.constEnd(); ++it ) {
    if ( (*it).name() == functionName )
      return true;
  }

  return false;
}

bool Class::isQObject() const
{
  Function::List::ConstIterator it;
  for ( it = d->mFunctions.constBegin(); it != d->mFunctions.constEnd(); ++it ) {
    if ( (*it).access() & Function::Signal || (*it).access() & Function::Slot )
      return true;
  }

  return false;
}

void Class::setDocs( const QString &str )
{
  d->mDocs = str;
}

QString Class::docs() const
{
  return d->mDocs;
}

void Class::addNestedClass( const Class &nestedClass )
{
  Class addedClass = nestedClass;
  addedClass.setParentClassName( name() );

  d->mNestedClasses.append( addedClass );
}

Class::List Class::nestedClasses() const
{
  return d->mNestedClasses;
}

QString Class::parentClassName() const
{
  return d->mParentClassName;
}

void Class::setParentClassName( const QString &parentClassName )
{
  d->mParentClassName = parentClassName;
}

QString Class::dPointerName() const
{
  return d->mDPointer;
}

////

// Returns what a class depends on: its base class(es) and any by-value member var
static QStringList dependenciesForClass( const Class& aClass, const QStringList& allClasses, const QStringList& excludedClasses )
{
    QStringList lst;
    Q_FOREACH( const Class& baseClass, aClass.baseClasses() ) {
        const QString baseName = baseClass.name();
        if ( !baseName.startsWith(QLatin1Char('Q')) && !excludedClasses.contains( baseName ) )
            lst.append( baseClass.name() );
    }
    if (!aClass.useDPointer())
    {
        Q_FOREACH( const MemberVariable& member, aClass.memberVariables() ) {
            const QString type = member.type();
            if ( allClasses.contains( type ) ) {
                lst.append(type);
            }
        }
    }

    return lst;
}

static bool allKnown( const QStringList& deps, const QStringList& classNames )
{
    Q_FOREACH(const QString& dep, deps) {
        if (!classNames.contains(dep)) {
            return false;
        }
    }
    return true;
}


/**
 * This method sorts a list of classes in a way that the base class
 * of a class, as well as the classes it use by value in member vars,
 * always appear before the class itself.
 */
static Class::List sortByDependenciesHelper( const Class::List &classes, const QStringList& excludedClasses )
{
    Class::List allClasses( classes );
    QStringList allClassNames;
    Q_FOREACH( const Class& c, classes )
        allClassNames.append( c.name() );

    Class::List retval;

    QStringList classNames;

    // copy all classes without dependencies
    Class::List::Iterator it;
    for ( it = allClasses.begin(); it != allClasses.end(); ++it ) {
      if ( dependenciesForClass( *it, allClassNames, excludedClasses ).isEmpty() ) {
        retval.append( *it );
        classNames.append( (*it).name() );

        it = allClasses.erase( it );
        it--;
      }
    }

    while ( allClasses.count() > 0 ) {
      const int currentCount = allClasses.count();
      // copy all classes which have a class from retval/classNames (i.e. already written out)
      // as base class - or as member variable
      for ( it = allClasses.begin(); it != allClasses.end(); ++it ) {

        const QStringList deps = dependenciesForClass( *it, allClassNames, excludedClasses );
        if ( allKnown( deps, classNames ) ) {
          retval.append( *it );
          classNames.append( (*it).name() );

          it = allClasses.erase( it );
          it--;
        }
      }
      if (allClasses.count() == currentCount) {
          // We didn't resolve anything this time around, so let's not loop forever
          qDebug() << "ERROR: Couldn't find class dependencies (base classes, member vars) for classes" << allClasses.classNames();
          Q_FOREACH(const Class& c, allClasses) {
              qDebug() << c.name() << "depends on" << dependenciesForClass( c, allClassNames, excludedClasses );
          }

          return retval;
      }
    }

    return retval;
}

void ClassList::sortByDependencies( const QStringList& excludedClasses )
{
    *this = sortByDependenciesHelper( *this, excludedClasses );
}

ClassList::iterator ClassList::findClass(const QString &name)
{
    ClassList::iterator it = begin();
    for (; it != end(); ++it)
        if ((*it).name() == name)
            break;
    return it;
}

QStringList KODE::ClassList::classNames() const
{
    QStringList names;
    ClassList::const_iterator it = begin();
    for (; it != end(); ++it)
        names.append((*it).name());
    return names;
}


void KODE::ClassList::addClass(const Class& cl)
{
    const QString qn = cl.qualifiedName();
    ClassList::iterator it = begin();
    for (; it != end(); ++it) {
        if ((*it).qualifiedName() == qn) {
            qWarning() << "ERROR: Already having a class called" << qn;
        }
    }

    *this += cl;
}

void KODE::Class::addDeclarationMacro(const QString &macro)
{
    d->mDeclMacros.append(macro);
}

QStringList KODE::Class::declarationMacros() const
{
    return d->mDeclMacros;
}

void KODE::Class::setNamespaceAndName( const QString& name )
{
    d->mName = name;
    d->mNameSpace.clear();
    while (d->mName.contains(QLatin1String("::"))) {
        const int pos = d->mName.indexOf(QLatin1String("::"));
        if (!d->mNameSpace.isEmpty())
            d->mNameSpace += QLatin1String(QLatin1String("::"));
        d->mNameSpace += d->mName.left(pos);
        d->mName = d->mName.mid(pos+2);
    }
}
