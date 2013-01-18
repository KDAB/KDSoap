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
#ifndef KDSOAPNAMESPACESPREFIXES_H
#define KDSOAPNAMESPACESPREFIXES_H

#include <QtCore/QMap>
#include <QtCore/QXmlStreamWriter>

#include "KDSoapClientInterface.h"

class KDSoapNamespacePrefixes : public QMap<QString /*ns*/, QString /*prefix*/>
{
public:
    void writeStandardNamespaces(QXmlStreamWriter& writer,
                                 KDSoapClientInterface::SoapVersion version = KDSoapClientInterface::SOAP1_1);

    void writeNamespace(QXmlStreamWriter& writer, const QString& ns, const QString& prefix) {
        //qDebug() << "writeNamespace" << ns << prefix;
        insert(ns, prefix);
        writer.writeNamespace(ns, prefix);
    }
    QString resolve(const QString& ns, const QString& localName) const {
        const QString prefix = value(ns);
        if (prefix.isEmpty()) {
            qWarning("ERROR: Namespace not found: %s (for localName %s)", qPrintable(ns), qPrintable(localName));
        }
        return prefix + QLatin1Char(':') + localName;
    }
};

#endif // KDSOAPNAMESPACESPREFIXES_H
