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
#ifndef KDSOAPNAMESPACEPREFIXES_P_H
#define KDSOAPNAMESPACEPREFIXES_P_H

#include <QtCore/QMap>
#include <QtCore/QXmlStreamWriter>

#include "KDSoapClientInterface.h"
#include "KDSoapMessageAddressingProperties.h"

class KDSoapNamespacePrefixes : public QMap<QString /*ns*/, QString /*prefix*/>
{
public:
    void writeStandardNamespaces(QXmlStreamWriter &writer,
                                 KDSoap::SoapVersion version = KDSoap::SOAP1_1,
                                 bool messageAddressingEnabled = false,
                                 KDSoapMessageAddressingProperties::KDSoapAddressingNamespace messageAddressingNamespace = KDSoapMessageAddressingProperties::Addressing200508
                                );

    void writeNamespace(QXmlStreamWriter &writer, const QString &ns, const QString &prefix)
    {
        //qDebug() << "writeNamespace" << ns << prefix;
        insert(ns, prefix);
        writer.writeNamespace(ns, prefix);
    }
    QString resolve(const QString &ns, const QString &localName) const
    {
        const QString prefix = value(ns);
        if (prefix.isEmpty()) {
            qWarning("ERROR: Namespace not found: %s (for localName %s)", qPrintable(ns), qPrintable(localName));
        }
        return prefix + QLatin1Char(':') + localName;
    }
};

#endif // KDSOAPNAMESPACESPREFIXES_H
