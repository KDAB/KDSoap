/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2015 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef KDSOAPENDPOINTREFERENCE_H
#define KDSOAPENDPOINTREFERENCE_H

#include "KDSoapGlobal.h"
#include "KDSoapValue.h"
#include <QSharedDataPointer>
#include <QString>

class KDSoapEndpointReferenceData;
/**
 * KDSoapEndpointReference is the abstraction class to hold
 * an Endpoint Reference properties.
 *
 * \see: https://www.w3.org/TR/ws-addr-core/#eprinfoset
 * \since 1.5
 */
class KDSOAP_EXPORT KDSoapEndpointReference
{
public:
    /**
     * Construct a KDSoapEndpointReference object with the given \p address
     * If no address is given, then an empty QString is taken
     */
    explicit KDSoapEndpointReference(const QString &address = QString());

    /**
     * Copy constructor of KDSoapEndpointReference
     */
    KDSoapEndpointReference(const KDSoapEndpointReference &other);

    /**
     * Copy the content of the KDSoapEndpointReference from \p other to the object
     */
    KDSoapEndpointReference &operator=(const KDSoapEndpointReference &other);

    /**
     * Destroys the object and frees up any resource used.
     */
    ~KDSoapEndpointReference();

    /**
     * Returns the address
     */
    QString address() const;

    /**
     * Sets the address of the endpoint reference
     * @param address address of the end point reference
     */
    void setAddress(const QString &address);

    /**
     * Return the reference parameters, which can be of any types
     * so we return a KDSoapValueList.
     */
    KDSoapValueList referenceParameters() const;

    /**
     * Sets the reference parameters
     */
    void setReferenceParameters(const KDSoapValueList &referenceParameters);

    /**
     * Return the meta data, which can be of any types
     * so we return a KDSoapValueList
     */
    KDSoapValueList metadata() const;

    /**
     * Sets the meta data
     */
    void setMetadata(const KDSoapValueList &metadata);

    /**
     * Return true when the address has not been set
     */
    bool isEmpty() const;

private:
    QSharedDataPointer<KDSoapEndpointReferenceData> d;
};

#endif // KDSOAPENDPOINTREFERENCE_H
