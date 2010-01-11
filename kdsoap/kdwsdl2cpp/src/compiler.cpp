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

#include <QCoreApplication>
#include <QFile>

#include <common/fileprovider.h>
#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include "converter.h"
#include "creator.h"
#include "settings.h"

#include "compiler.h"

using namespace KWSDL;

Compiler::Compiler()
  : QObject( 0 )
{
}

void Compiler::run()
{
  download();
}

void Compiler::download()
{
  FileProvider provider;

  QString fileName;
  if ( provider.get( Settings::self()->wsdlUrl(), fileName ) ) {
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
      qDebug( "Unable to download schema file %s", qPrintable( Settings::self()->wsdlUrl() ) );
      provider.cleanUp();
      return;
    }

    QXmlInputSource source( &file );
    QXmlSimpleReader reader;
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", true );

    QDomDocument document( "KWSDL" );

    QString errorMsg;
    int errorLine, errorCol;
    QDomDocument doc;
    if ( !doc.setContent( &source, &reader, &errorMsg, &errorLine, &errorCol ) ) {
      qDebug( "%s at (%d,%d)", qPrintable( errorMsg ), errorLine, errorCol );
      return;
    }

    parse( doc.documentElement() );

    provider.cleanUp();
  }
}

void Compiler::parse( const QDomElement &element )
{
  NSManager namespaceManager;
  MessageHandler messageHandler;
  ParserContext context;
  context.setNamespaceManager( &namespaceManager );
  context.setMessageHandler( &messageHandler );
  context.setDocumentBaseUrl( Settings::self()->wsdlBaseUrl() );

  Definitions definitions;
  definitions.loadXML( &context, element );

  mWSDL.setDefinitions( definitions );
  mWSDL.setNamespaceManager( namespaceManager );

  create();
}

void Compiler::create()
{
  KWSDL::Converter converter;
  converter.setWSDL( mWSDL );

  converter.convert();

  KWSDL::Creator creator;
  creator.create( converter.classes() );

  QCoreApplication::exit( 0 );
}

#include "moc_compiler.cpp"
