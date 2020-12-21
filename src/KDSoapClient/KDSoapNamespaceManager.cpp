/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/
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

QString KDSoapNamespaceManager::soapSecurityExtention()
{
    return QString::fromLatin1("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd");
}

QString KDSoapNamespaceManager::soapSecurityUtility()
{
    return QString::fromLatin1("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd");
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
