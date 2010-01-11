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

#include <QDir>
#include <QDomDocument>
#include <QFile>

#include "settings.h"

Settings *Settings::mSelf = 0;

// TODO Q_GLOBAL_STATIC static K3StaticDeleter<Settings> settingsDeleter;

Settings::Settings()
  : mTransport( KDETransport )
{
  mOutputDirectory = QDir::current().path();
  mOutputFileName = "kwsdl_generated";
}

Settings::~Settings()
{
}

Settings* Settings::self()
{
  if ( !mSelf )
#ifdef KDAB_TEMP
    settingsDeleter.setObject( mSelf, new Settings );
#else
    mSelf = new Settings;
#endif

  return mSelf;
}

bool Settings::load( const QString &fileName )
{
  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    qDebug( "Settings::load: Can't open %s.", qPrintable( file.fileName() ) );
    return false;
  }

  QString errorMsg;
  int line, column;
  QDomDocument document;
  if ( !document.setContent( &file, &errorMsg, &line, &column ) ) {
    qDebug( "Settings::load: Can't parse configuration '%s (%d:%d)'.",
            qPrintable( errorMsg ), line, column );
    return false;
  }

  QDomElement element = document.documentElement();
  if ( element.tagName() != "kwsdlcfg" ) {
    qDebug( "Settings::load: Unknown xml format." );
    return false;
  }

  element = element.firstChildElement();
  while ( !element.isNull() ) {
    if ( element.tagName() == "wsdlUrl" ) {
      setWsdlUrl( element.text() );
    } else if ( element.tagName() == "outputFileName" ) {
      setOutputFileName( element.text() );
    } else if ( element.tagName() == "outputDirectory" ) {
      setOutputDirectory( element.text() );
    } else if ( element.tagName() == "namespaceMapping" ) {
      const QString prefix = element.attribute( "prefix" );
      const QString uri = element.attribute( "uri" );

      if ( !prefix.isEmpty() && !uri.isEmpty() )
        mNamespaceMapping.insert( uri, prefix );
    } else if ( element.tagName() == "transport" ) {
      const QString data = element.text();
      if ( data == "KDE" )
        setTransport( KDETransport );
      else if ( data == "Qt" )
        setTransport( QtTransport );
      else if ( data == "Custom" )
        setTransport( CustomTransport );
    } else {
      qDebug( "Settings::load: Unknown xml element %s.", qPrintable( element.tagName() ) );
      return false;
    }

    element = element.nextSiblingElement();
  }

  return true;
}

void Settings::setWsdlUrl( const QString &wsdlUrl )
{
  mWsdlUrl = wsdlUrl;

  if ( QDir::isRelativePath( mWsdlUrl ) )
    mWsdlUrl = QDir::current().path() + '/' + mWsdlUrl;
}

QString Settings::wsdlUrl() const
{
  return mWsdlUrl;
}

QString Settings::wsdlBaseUrl() const
{
  return mWsdlUrl.left( mWsdlUrl.lastIndexOf( '/' ) );
}

QString Settings::wsdlFileName() const
{
  return mWsdlUrl.mid( mWsdlUrl.lastIndexOf( '/' ) + 1 );
}

void Settings::setOutputFileName( const QString &outputFileName )
{
  mOutputFileName = outputFileName;
}

QString Settings::outputFileName() const
{
  return mOutputFileName;
}

void Settings::setOutputDirectory( const QString &outputDirectory )
{
  mOutputDirectory = outputDirectory;

  if ( !mOutputDirectory.endsWith( "/" ) )
    mOutputDirectory.append( "/" );
}

QString Settings::outputDirectory() const
{
  return mOutputDirectory;
}

void Settings::setNamespaceMapping( const NSMapping &namespaceMapping )
{
  mNamespaceMapping = namespaceMapping;
}

Settings::NSMapping Settings::namespaceMapping() const
{
  return mNamespaceMapping;
}

void Settings::setTransport( Transport transport )
{
  mTransport = transport;
}

Settings::Transport Settings::transport() const
{
  return mTransport;
}
