/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KSOAPMESSAGEREADER_P_H
#define KSOAPMESSAGEREADER_P_H

#include "KDSoapMessage.h"

class KDSOAP_EXPORT KDSoapMessageReader
{
public:
    enum XmlError {
        NoError=0,
        ParseError,
        PrematureEndOfDocumentError
    };

    KDSoapMessageReader();

    XmlError xmlToMessage(const QByteArray& data, KDSoapMessage* pParsedMessage, QString* pMessageNamespace, KDSoapHeaders* pRequestHeaders) const;
};

#endif
