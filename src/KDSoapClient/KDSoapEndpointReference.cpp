/****************************************************************************
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
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

#include "KDSoapEndpointReference.h"

#include <QDebug>

class KDSoapEndpointReferenceData : public QSharedData
{
public:
    KDSoapEndpointReferenceData() {}

    QString m_address;
    KDSoapValueList m_metadata;
    KDSoapValueList m_referenceParameters;
};

KDSoapEndpointReference::KDSoapEndpointReference(const QString &address)
    : d(new KDSoapEndpointReferenceData)
{
    d->m_address = address;
}

KDSoapEndpointReference::KDSoapEndpointReference(const KDSoapEndpointReference &other)
    : d(other.d)
{
}

KDSoapEndpointReference &KDSoapEndpointReference::operator =(const KDSoapEndpointReference &other)
{
    d = other.d;
    return *this;
}

KDSoapEndpointReference::~KDSoapEndpointReference()
{
}

QString KDSoapEndpointReference::address() const
{
    return d->m_address;
}

void KDSoapEndpointReference::setAddress(const QString &address)
{
    d->m_address = address;
}

KDSoapValueList KDSoapEndpointReference::metadata() const
{
    return d->m_metadata;
}

void KDSoapEndpointReference::setMetadata(const KDSoapValueList &metadata)
{
    d->m_metadata = metadata;
}

bool KDSoapEndpointReference::isEmpty() const
{
    return d->m_address.isEmpty();
}

KDSoapValueList KDSoapEndpointReference::referenceParameters() const
{
    return d->m_referenceParameters;
}

void KDSoapEndpointReference::setReferenceParameters(const KDSoapValueList &referenceParameters)
{
    d->m_referenceParameters = referenceParameters;
}

