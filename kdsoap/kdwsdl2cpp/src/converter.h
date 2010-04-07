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

#ifndef KWSDL_CONVERTER_H
#define KWSDL_CONVERTER_H

#include <common/nsmanager.h>
#include <libkode/class.h>
#include <schema/parser.h>
#include <wsdl/wsdl.h>

#include "namemapper.h"
#include "typemap.h"

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

    void convertAttribute( const XSD::Attribute* );
    void createAttributeSerializer( const XSD::Attribute* );

    //void convertElement( const XSD::Element* );
    //void createElementSerializer( const XSD::Element* );

    // Client Stub
    void convertClientService();
    void convertClientCall( const Operation&, const Binding&, KODE::Class& );
    void convertClientInputMessage( const Operation&, const Param&, const Binding&, KODE::Class& );
    void convertClientOutputMessage( const Operation&, const Param&, const Binding&, KODE::Class& );
    void clientAddArguments( KODE::Function& callFunc, const Message& message, KODE::Class &newClass );
    bool clientAddAction( KODE::Code& code, const Binding &binding, const QString& operationName );
    void clientGenerateMessage( KODE::Code& code, const Binding& binding, const Message& message );

    // Server Stub
    void convertServerService();

    // Utils
    void createSoapUtils();

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
