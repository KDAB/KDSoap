/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 David Faure <dfaure@kdab.com>

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
#include <QtCore/QDebug>

#include "function.h"

using namespace KODE;

class Function::Argument::ArgumentPrivate
{
  public:
    QString declaration;
    QString defaultArgument;
};

Function::Argument::Argument( const QString &declaration,
  const QString &defaultArgument )
  : d( new ArgumentPrivate )
{
  d->declaration = declaration;
  d->defaultArgument = defaultArgument;
}

QString Function::Argument::headerDeclaration() const
{
  if ( d->defaultArgument.isEmpty() ) {
    return d->declaration;
  } else {
    return d->declaration + QLatin1String(" = ") + d->defaultArgument;
  }
}

QString Function::Argument::bodyDeclaration() const
{
  return d->declaration;
}


class Function::FunctionPrivate
{
  public:
    FunctionPrivate()
      : mAccess( Public ), mIsConst( false ), mIsStatic( false ), mVirtualMode( NotVirtual )
    {
    }

    int mAccess;
    bool mIsConst;
    bool mIsStatic;
    QString mReturnType;
    QString mName;
    Argument::List mArguments;
    QStringList mInitializers;
    QString mBody;
    QString mDocs;
    Function::VirtualMode mVirtualMode;
};

Function::Function()
  : d( new FunctionPrivate )
{
}

Function::Function( const Function &other )
  : d( new FunctionPrivate )
{
  *d = *other.d;
}

Function::Function( const QString &name, const QString &returnType,
                    int access, bool isStatic )
  : d( new FunctionPrivate )
{
  d->mAccess = access;
  d->mIsStatic = isStatic;
  d->mName = name;
  setReturnType(returnType);
}

Function::~Function()
{
  delete d;
}

Function& Function::operator=( const Function &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void Function::setConst( bool isConst )
{
  d->mIsConst = isConst;
}

bool Function::isConst() const
{
  return d->mIsConst;
}

void Function::setStatic( bool isStatic )
{
  d->mIsStatic = isStatic;
}

bool Function::isStatic() const
{
  return d->mIsStatic;
}

void Function::addArgument( const Function::Argument &argument )
{
  d->mArguments.append( argument );
}

void Function::addArgument( const QString &argument )
{
  d->mArguments.append( Argument( argument ) );
}

void Function::setArgumentString( const QString &argumentString )
{
  d->mArguments.clear();

  const QStringList arguments = argumentString.split( QLatin1String(",") );
  QStringList::ConstIterator it;
  for ( it = arguments.constBegin(); it != arguments.constEnd(); ++it ) {
    addArgument( *it );
  }
}

Function::Argument::List Function::arguments() const
{
  return d->mArguments;
}

void Function::addInitializer( const QString &initializer )
{
  d->mInitializers.append( initializer );
}

QStringList Function::initializers() const
{
  return d->mInitializers;
}

void Function::setBody( const QString &body )
{
  d->mBody = body;
}

void Function::setBody( const Code &body )
{
  d->mBody = body.text();
}

void Function::addBodyLine( const QString &bodyLine )
{
  d->mBody.append( bodyLine );
  if ( bodyLine.right( 1 ) != QLatin1String("\n") )
    d->mBody.append( QLatin1Char('\n') );
}

QString Function::body() const
{
  return d->mBody;
}

void Function::setAccess( int access )
{
  d->mAccess = access;
}

int Function::access() const
{
  return d->mAccess;
}

QString Function::accessAsString() const
{
  QString access;

  if ( d->mAccess & Public )
    access = QLatin1String("public");
  if ( d->mAccess & Protected )
    access = QLatin1String("protected");
  if ( d->mAccess & Private )
    access = QLatin1String("private");

  if ( d->mAccess & Signal )
    access = QLatin1String("Q_SIGNALS");
  if ( d->mAccess & Slot )
    access += QLatin1String(" Q_SLOTS");

  return access;
}

void Function::setReturnType( const QString &returnType )
{
  Q_ASSERT(returnType != QLatin1String("*"));
  d->mReturnType = returnType;
}

QString Function::returnType() const
{
  return d->mReturnType;
}

void Function::setName( const QString &name )
{
  d->mName = name;
}

QString Function::name() const
{
  return d->mName;
}

void Function::setDocs( const QString &docs )
{
  d->mDocs = docs;
}

QString Function::docs() const
{
  return d->mDocs;
}

bool Function::hasArguments() const
{
  return !d->mArguments.isEmpty();
}

void Function::setVirtualMode( Function::VirtualMode v )
{
  d->mVirtualMode = v;
}

Function::VirtualMode Function::virtualMode() const
{
  return d->mVirtualMode;
}

QDebug operator<<(QDebug dbg, const Function &func)
{
    dbg << func.name();
    return dbg;
}
