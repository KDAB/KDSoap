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

#include "function.h"

using namespace KODE;

class Function::FunctionPrivate
{
  public:
    FunctionPrivate()
      : mAccess( Public ), mIsConst( false ), mIsStatic( false )
    {
    }

    class Argument
    {
    public:
        Argument(const QString& name, const QString& value)
            : mArgName(name), mArgDefaultValue(value) {}
        QString mArgName;
        QString mArgDefaultValue;
    };

    int mAccess;
    bool mIsConst;
    bool mIsStatic;
    QString mReturnType;
    QString mName;
    QList<Argument> mArguments;
    QStringList mInitializers;
    QString mBody;
    QString mDocs;
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
  d->mReturnType = returnType;
  d->mName = name;
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

void Function::addArgument( const QString &argument, const QString& defaultValue )
{
  d->mArguments.append( FunctionPrivate::Argument(argument, defaultValue) );
}

void Function::setArgumentString( const QString &argumentString )
{
  d->mArguments.clear();

  const QStringList arguments = argumentString.split( "," );
  QStringList::ConstIterator it;
  for ( it = arguments.constBegin(); it != arguments.constEnd(); ++it ) {
    addArgument( *it );
  }
}

QStringList Function::arguments( bool forImplementation ) const
{
  QStringList lst;
  Q_FOREACH(const FunctionPrivate::Argument& arg, d->mArguments) {
      QString argStr = arg.mArgName;
      if (!forImplementation && !arg.mArgDefaultValue.isEmpty()) {
          argStr += " = " + arg.mArgDefaultValue;
      }
      lst << argStr;
  }

  return lst;
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
  if ( bodyLine.right( 1 ) != "\n" )
    d->mBody.append( '\n' );
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
    access = "public";
  if ( d->mAccess & Protected )
    access = "protected";
  if ( d->mAccess & Private )
    access = "private";

  if ( d->mAccess & Signal )
    access = "signals";
  if ( d->mAccess & Slot )
    access += " slots";

  return access;
}

void Function::setReturnType( const QString &returnType )
{
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

bool KODE::Function::hasArguments() const
{
  return !d->mArguments.isEmpty();
}
