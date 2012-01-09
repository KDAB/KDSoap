/****************************************************************************
** Copyright (C) 2010-2012 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"

void KDSoapNamespacePrefixes::writeStandardNamespaces(QXmlStreamWriter& writer)
{
    writeNamespace(writer, KDSoapNamespaceManager::soapEnvelope(), QLatin1String("soap"));
    writeNamespace(writer, KDSoapNamespaceManager::soapEncoding(), QLatin1String("soap-enc"));
    writeNamespace(writer, KDSoapNamespaceManager::xmlSchema1999(), QLatin1String("xsd"));
    writeNamespace(writer, KDSoapNamespaceManager::xmlSchemaInstance1999(), QLatin1String("xsi"));

    // Also insert known variants
    insert(KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("xsd"));
    insert(KDSoapNamespaceManager::xmlSchemaInstance2001(), QString::fromLatin1("xsi"));
}
