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

#include <wsdl/abstractbinding.h>

#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include "bindingoperation.h"

using namespace KWSDL;

BindingOperation::BindingOperation()
{
}

BindingOperation::BindingOperation( const QString &nameSpace )
  : Element( nameSpace )
{
}

BindingOperation::~BindingOperation()
{
}

void BindingOperation::setName( const QString &name )
{
  mName = name;
}

QString BindingOperation::name() const
{
  return mName;
}

void BindingOperation::loadXML( AbstractBinding *binding, ParserContext *context, const QDomElement &element )
{
  mName = element.attribute( "name" );
  if ( mName.isEmpty() )
    context->messageHandler()->warning( "BindingOperation: 'name' required" );

  binding->parseOperation( context, mName, element );

  QDomElement child = element.firstChildElement();
  while ( !child.isNull() ) {
    QName tagName = child.tagName();
    if ( tagName.localName() == "input" ) {
      binding->parseOperationInput( context, mName, child );
    } else if ( tagName.localName() == "output" ) {
      binding->parseOperationOutput( context, mName, child );
    } else if ( tagName.localName() == "fault" ) {
      binding->parseOperationFault( context, mName, child );
    }

    child = child.nextSiblingElement();
  }
}

void BindingOperation::saveXML( const AbstractBinding *binding, ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElement( "operation" );
  parent.appendChild( element );

  if ( !mName.isEmpty() )
    element.setAttribute( "name", mName );
  else
    context->messageHandler()->warning( "BindingOperation: 'name' required" );

  binding->synthesizeOperation( context, mName, document, element );

  QDomElement inputElement = document.createElement( "input" );
  element.appendChild( inputElement );
  binding->synthesizeOperationInput( context, mName, document, inputElement );

  QDomElement outputElement = document.createElement( "output" );
  element.appendChild( outputElement );
  binding->synthesizeOperationOutput( context, mName, document, outputElement );

  QDomElement faultElement = document.createElement( "fault" );
  element.appendChild( faultElement );
  binding->synthesizeOperationFault( context, mName, document, faultElement );
}
