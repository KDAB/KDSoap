/****************************************************************************
** Copyright (C) 2010-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

/*
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

#ifndef KWSDL_CONVERTER_H
#define KWSDL_CONVERTER_H

#include <common/nsmanager.h>
#include <libkode/class.h>
#include <schema/parser.h>
#include <wsdl/wsdl.h>

#include "namemapper.h"
#include "typemap.h"

#ifdef NDEBUG
#define COMMENT
#else
#define COMMENT "// " __FILE__ ":"  + QString::number(__LINE__)
#endif

namespace KWSDL {

class Converter
{
  public:
    Converter();

    void setWSDL( const WSDL &wsdl );

    void convert();

    KODE::Class::List classes() const;

  private:
    void cleanupUnusedTypes();
    void convertTypes();

    void convertComplexType( const XSD::ComplexType* );
    void createComplexTypeSerializer( KODE::Class&, const XSD::ComplexType* );

    void convertSimpleType( const XSD::SimpleType*, const XSD::SimpleType::List& simpleTypeList );
    void createSimpleTypeSerializer( KODE::Class&, const XSD::SimpleType*, const XSD::SimpleType::List& simpleTypeList );

    // Client Stub
    void convertClientService();
    void convertClientCall( const Operation&, const Binding&, KODE::Class& );
    void convertClientInputMessage( const Operation&, const Binding&, KODE::Class& );
    void convertClientOutputMessage( const Operation&, const Binding&, KODE::Class& );
    void clientAddOneArgument( KODE::Function& callFunc, const Part& part, KODE::Class &newClass );
    void clientAddArguments( KODE::Function& callFunc, const Message& message, KODE::Class &newClass );
    bool clientAddAction( KODE::Code& code, const Binding &binding, const QString& operationName );
    void clientGenerateMessage( KODE::Code& code, const Binding& binding, const Message& message, const Operation& operatoin );
    void clientAddMessageArgument( KODE::Code& code, const SoapBinding::Style& bindingStyle, const Part& part );
    void createHeader( const SoapBinding::Header& header, KODE::Class& newClass );
    KODE::Code appendElementArg( const QName& type, const QName& elementType, const QString& name, const QString& localVariableName, const QByteArray& varName );
    KODE::Code demarshalVar( const QName& type, const QName& elementType, const QString& variableName, const QString& typeName, const QString& soapValueVarName = "val" ) const;
    KODE::Code demarshalArrayVar( const QName& type, const QString& variableName, const QString& typeName ) const;

    // Server Stub
    void convertServerService();

    WSDL mWSDL;

    KODE::Class::List mClasses;
    KODE::Class mQObject;

    NameMapper mNameMapper;
    TypeMap mTypeMap;
    NSManager mNSManager;
};

}

QString upperlize( const QString& );
QString lowerlize( const QString& );

static QName XmlAnyType( "http://www.w3.org/2001/XMLSchema", "any" );

#endif
