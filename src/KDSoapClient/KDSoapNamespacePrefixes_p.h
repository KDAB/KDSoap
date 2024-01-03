/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
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
    void writeStandardNamespaces(QXmlStreamWriter &writer, KDSoap::SoapVersion version = KDSoap::SOAP1_1, bool messageAddressingEnabled = false,
                                 KDSoapMessageAddressingProperties::KDSoapAddressingNamespace messageAddressingNamespace =
                                     KDSoapMessageAddressingProperties::Addressing200508);

    void writeNamespace(QXmlStreamWriter &writer, const QString &ns, const QString &prefix)
    {
        // qDebug() << "writeNamespace" << ns << prefix;
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
