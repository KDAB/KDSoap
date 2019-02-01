/****************************************************************************
** Copyright (C) 2010-2018 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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
#include "KDSoapNamespaceManager.h"

KDSoapNamespaceManager::KDSoapNamespaceManager()
{
}

QString KDSoapNamespaceManager::xmlSchema1999()
{
    return QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
}

QString KDSoapNamespaceManager::xmlSchema2001()
{
    return QString::fromLatin1("http://www.w3.org/2001/XMLSchema");
}

QString KDSoapNamespaceManager::xmlSchemaInstance1999()
{
    return QString::fromLatin1("http://www.w3.org/1999/XMLSchema-instance");
}

QString KDSoapNamespaceManager::xmlSchemaInstance2001()
{
    return QString::fromLatin1("http://www.w3.org/2001/XMLSchema-instance");
}

QString KDSoapNamespaceManager::soapEnvelope()
{
    return QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
}

QString KDSoapNamespaceManager::soapEnvelope200305()
{
    return QString::fromLatin1("http://www.w3.org/2003/05/soap-envelope");
}

QString KDSoapNamespaceManager::soapEncoding()
{
    return QString::fromLatin1("http://schemas.xmlsoap.org/soap/encoding/");
}

QString KDSoapNamespaceManager::soapEncoding200305()
{
    return QString::fromLatin1("http://www.w3.org/2003/05/soap-encoding");
}

QString KDSoapNamespaceManager::soapMessageAddressing()
{
    return QString::fromLatin1("http://www.w3.org/2005/08/addressing");
}

QString KDSoapNamespaceManager::soapMessageAddressing200303()
{
    return QString::fromLatin1("http://schemas.xmlsoap.org/ws/2003/03/addressing");
}

QString KDSoapNamespaceManager::soapMessageAddressing200403()
{
    return QString::fromLatin1("http://schemas.xmlsoap.org/ws/2004/03/addressing");
}

QString KDSoapNamespaceManager::soapMessageAddressing200408()
{
    return QString::fromLatin1("http://schemas.xmlsoap.org/ws/2004/08/addressing");
}
