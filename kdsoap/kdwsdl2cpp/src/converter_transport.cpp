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

#include "converter.h"

using namespace KWSDL;

void Converter::createKDETransport()
{
  KODE::Class transport( "Transport" );
  transport.addBaseClass( mQObject );
  transport.addHeaderInclude( "QObject" );
  transport.addHeaderInclude( "kio/job.h" );

  transport.addInclude( "kdebug.h" );

  KODE::MemberVariable url( "url", "QString" );
  transport.addMemberVariable( url );

  KODE::MemberVariable slotDataVar( "data", "QByteArray" );
  transport.addMemberVariable( slotDataVar );

  // ctor
  KODE::Function ctor( "Transport" );
  ctor.addArgument( "const QString &url" );
  ctor.setBody( url.name() + " = url;" );

  transport.addFunction( ctor );

  // query
  KODE::Function query( "query", "void" );
  query.addArgument( "const QString &xml" );
  query.addArgument( "const QString &header" );

  KODE::Code queryCode;
  queryCode += slotDataVar.name() + ".truncate( 0 );";
  queryCode.newLine();
  queryCode += "QByteArray postData;";
  queryCode += "QDataStream stream( &postData, QIODevice::WriteOnly );";
  queryCode += "stream.writeRawData( xml.toUtf8(), xml.toUtf8().length() );";
  queryCode.newLine();
  queryCode += "KIO::TransferJob* job = KIO::http_post( KUrl( " + url.name() + " ), postData, KIO::HideProgressInfo );";
  queryCode += "if ( !job ) {";
  queryCode.indent();
  queryCode += "kWarning() << \"Unable to create KIO job for \" <<" + url.name() +";";
  queryCode += "return;";
  queryCode.unindent();
  queryCode += '}';
  queryCode.newLine();
  queryCode += "job->addMetaData( \"UserAgent\", \"KWSDL\" );";
  queryCode += "job->addMetaData( \"content-type\", \"Content-Type: application/xml; charset=utf-8\" );";
  queryCode += "if ( !header.isEmpty() ) {";
  queryCode.indent();
  queryCode += "job->addMetaData( \"customHTTPHeader\", \"SOAPAction:\" + header );";
  queryCode.unindent();
  queryCode += '}';
  queryCode.newLine();
  queryCode += "connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );";
  queryCode += "connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );";

  query.setBody( queryCode );

  transport.addFunction( query );

  // signal
  KODE::Function result( "result", "void", KODE::Function::Signal );
  result.addArgument( "const QString &xml" );

  KODE::Function error( "error", "void", KODE::Function::Signal );
  error.addArgument( "const QString &msg" );

  transport.addFunction( result );
  transport.addFunction( error );

  // data slot
  KODE::Function slotData( "slotData", "void", KODE::Function::Private | KODE::Function::Slot );

  slotData.addArgument( "KIO::Job*" );
  slotData.addArgument( "const QByteArray &data" );

  KODE::Code slotDataCode;
  slotDataCode += "unsigned int oldSize = " + slotDataVar.name() + ".size();";
  slotDataCode += slotDataVar.name() + ".resize( oldSize + data.size() );";
  slotDataCode += "memcpy( " + slotDataVar.name() + ".data() + oldSize, data.data(), data.size() );";

  slotData.setBody( slotDataCode );

  transport.addFunction( slotData );

  // result slot
  KODE::Function slotResult( "slotResult", "void", KODE::Function::Private | KODE::Function::Slot );
  slotResult.addArgument( "KJob* job" );

  KODE::Code slotResultCode;
  slotResultCode += "if ( job->error() != 0 ) {";
  slotResultCode.indent();
  slotResultCode += "emit error( job->errorText() );";
  slotResultCode += "return;";
  slotResultCode.unindent();
  slotResultCode += '}';
  slotResultCode.newLine();

  slotResultCode += "emit result( QString::fromUtf8( " + slotDataVar.name() + ".data(), " + slotDataVar.name() + ".size() ) );";

  slotResult.setBody( slotResultCode );

  transport.addFunction( slotResult );

  mClasses.append( transport );
}

void Converter::createQtTransport()
{
  KODE::Class transport( "Transport" );
  transport.addBaseClass( mQObject );
  transport.addHeaderInclude( "QBuffer" );
  transport.addHeaderInclude( "QByteArray" );
  transport.addHeaderInclude( "QObject" );
  transport.addHeaderInclude( "QHttp" );
  transport.addHeaderInclude( "QUrl" );

  // member variables
  KODE::MemberVariable bufferVar( "buffer", "QBuffer" );
  transport.addMemberVariable( bufferVar );

  KODE::MemberVariable dataVar( "data", "QByteArray" );
  transport.addMemberVariable( dataVar );

  KODE::MemberVariable httpVar( "http", "QHttp*" );
  transport.addMemberVariable( httpVar );

  KODE::MemberVariable urlVar( "url", "QUrl" );
  transport.addMemberVariable( urlVar );

  KODE::MemberVariable idVar( "id", "int" );
  transport.addMemberVariable( idVar );

  // functions
  KODE::Function ctor( "Transport" );
  ctor.addArgument( "const QString &url" );
  ctor.addInitializer( "QObject( 0 )" );
  ctor.addInitializer( urlVar.name() + "( url )" );

  KODE::Function query( "query", "void" );
  query.addArgument( "const QString &message" );
  query.addArgument( "const QString &headerStr" );

  KODE::Function resultSignal( "result", "void", KODE::Function::Signal );
  resultSignal.addArgument( "const QString &result" );

  KODE::Function errorSignal( "error", "void", KODE::Function::Signal );
  errorSignal.addArgument( "const QString &msg" );

  KODE::Function finishedSlot( "finished", "void", KODE::Function::Slot | KODE::Function::Private );
  finishedSlot.addArgument( "int id" );
  finishedSlot.addArgument( "bool errorOccurred" );

  // codes
  KODE::Code code;

  code += "QUrl server( url );";
  code.newLine();
  code += httpVar.name() + " = new QHttp( this );";
  code += httpVar.name() + "->setHost( server.host(), server.port( 80 ) );";
  code.newLine();
  code += "connect( " + httpVar.name() + ", SIGNAL( requestFinished( int, bool ) ), this, SLOT( " + finishedSlot.name() + "( int, bool ) ) );";
  ctor.setBody( code );

  code.clear();
  code += dataVar.name() + ".clear();";
  code += bufferVar.name() + ".setBuffer( &" + dataVar.name() + " );";
  code.newLine();
  code += "QHttpRequestHeader header;";
  code += "header.setRequest( \"POST\", " + urlVar.name() + ".path() );";
  code += "header.addValue( \"Connection\", \"Keep-Alive\" );";
  code += "header.addValue( \"Content-Type\", \"application/xml; charset=utf-8\" );";
  code += "header.addValue( \"Host\", QUrl( " + urlVar.name() + " ).host() );";
  code.newLine();
  code += "if ( !headerStr.isEmpty() )";
  code.indent();
  code += "header.addValue( \"SOAPAction\", headerStr );";
  code.unindent();
  code.newLine();
  code += "QUrl server( " + urlVar.name() + " );";
  code += "if ( server.port( 80 ) != 80 )";
  code.indent();
  code += "header.setValue( \"Host\", server.host() + \":\" + QString::number( server.port() ) );";
  code.unindent();
  code += "else";
  code.indent();
  code += "header.setValue( \"Host\", server.host() );";
  code.unindent();
  code.newLine();
  code += idVar.name() + " = " + httpVar.name() + "->request( header, message.toUtf8(), &" + bufferVar.name() + " );";
  query.setBody( code );

  code.clear();
  code += "if ( id != " + idVar.name() + " )";
  code.indent();
  code += "return;";
  code.unindent();
  code.newLine();
  code += "if ( errorOccurred )";
  code.indent();
  code += "emit " + errorSignal.name() + "( " + httpVar.name() + "->errorString() );";
  code.unindent();
  code += "else";
  code.indent();
  code += "emit " + resultSignal.name() + "( QString::fromUtf8( " + dataVar.name() + " ) );";
  code.unindent();
  finishedSlot.setBody( code );

  transport.addFunction( ctor );
  transport.addFunction( query );
  transport.addFunction( resultSignal );
  transport.addFunction( errorSignal );
  transport.addFunction( finishedSlot );

  mClasses.append( transport );
}

void Converter::createCustomTransport()
{
  mSerializer.addHeaderInclude( "transport.h" );
}
