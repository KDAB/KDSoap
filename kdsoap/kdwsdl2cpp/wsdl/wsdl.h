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

#ifndef KWSDL_WSDL_H
#define KWSDL_WSDL_H

#include <common/nsmanager.h>
#include <wsdl/definitions.h>
#include <kode_export.h>

namespace KWSDL {

class KWSDL_EXPORT WSDL
{
  public:
    WSDL();
    ~WSDL();

    void setDefinitions( const Definitions &definitions );
    Definitions definitions() const;

    void setNamespaceManager( const NSManager &namespaceManager );
    const NSManager& namespaceManager() const;

    Binding findBinding( const QName &bindingName ) const;
    BindingOperation findBindingOperation( const Binding &binding, const QString &operationName );
    PortType findPortType( const QName &portTypeName ) const;
    Message findMessage( const QName &messageName ) const;
    XSD::Element findElement( const QName &elementName ) const;

  private:
    Definitions mDefinitions;
    NSManager mNamespaceManager;
};

}

#endif
