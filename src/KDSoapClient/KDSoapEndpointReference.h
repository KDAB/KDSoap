/****************************************************************************
** Copyright (C) 2010-2015 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDSOAPENDPOINTREFERENCE_H
#define KDSOAPENDPOINTREFERENCE_H

#include "KDSoapGlobal.h"
#include <QString>

// Class following WS-Addressing specification
/*
 * \see: http://www.w3.org/TR/ws-addr-core/#eprinfoset
*/

class KDSOAP_EXPORT KDSoapEndpointReference
{
  public:

    KDSoapEndpointReference();
    ~KDSoapEndpointReference();

    //void loadXML( ParserContext *context, const QDomElement &element );

    QString address() const;
    void setAddress(const QString &address);

  private:
    QString m_address; // mandatory //see  predefined addresses : anonymous an none"

    //    <wsa:EndpointReference>
//        <wsa:Address>xs:anyURI</wsa:Address>
//        <wsa:ReferenceParameters>xs:any*</wsa:ReferenceParameters> ?
//        <wsa:Metadata>xs:any*</wsa:Metadata>?
//    </wsa:EndpointReference>
// reference properties : *
// reference parameters : *
// metadata : *
// qname of the port type : 0..1
// policy : 0*

};

#endif // KDSOAPENDPOINTREFERENCE_H

