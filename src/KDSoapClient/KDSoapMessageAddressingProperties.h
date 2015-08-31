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
#ifndef KDSOAPMESSAGEADDRESSINGPROPERTIES_H
#define KDSOAPMESSAGEADDRESSINGPROPERTIES_H

#include <QtCore/QSharedDataPointer>
#include "KDSoapGlobal.h"
#include <QPair>

#include "KDSoapValue.h"
QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

class KDSoapNamespacePrefixes;
class KDSoapMessageAddressingPropertiesData;

typedef QPair<QString, QString> Relationship;

/**
 * The KDSoapMessageAddressingProperties class is the abstraction of the
 * WS-Addressing specification. This specification set up information within
 * soap envelope header. This class is meant to be filled with the data you want
 * to have in the soap header and associate to a given message using
 * \see KDSoapMessage::setMessageAddressingProperties
 *
 * Important: This class does not ensure any kind of validation to the data being passed to it
 * \since 1.5
 */

class KDSOAP_EXPORT KDSoapMessageAddressingProperties
{
public:

    /**
     * This enum contains all the predefined addresses used within
     * ws addressing specification, it is meant to be used with predefinedAddress
     * helper function to retrieve the uri as a QString
     * \see predefinedAddressToString
     */
    enum KDSoapAddressingPredefinedAddress {
        None,
        Anonymous,
        Reply,
        Unspecified
    };

    /**
     * Constructs an empty KDSoapMessageAddressingProperties object.
     */
    KDSoapMessageAddressingProperties();

    /**
     * Destructs the KDSoapMessageAddressingProperties object.
     */
    ~KDSoapMessageAddressingProperties();

    /**
     * Constructs a copy of the KDSoapMessageAddressingProperties object given by \p other.
     */
    KDSoapMessageAddressingProperties(const KDSoapMessageAddressingProperties &other);

    /**
     * Copies the contents of the object given by \p other.
     */
    KDSoapMessageAddressingProperties &operator =(const KDSoapMessageAddressingProperties &other);

    /**
     * Returns the destination address, it should match the EndpointReference given from WSDL
     */
    QString destination() const;

    /**
     * Sets the destination address, where the message will be sent to
     */
    void setDestination(const QString &destination);

    /**
     * Returns the action uri, which is the semantic of the message
     */
    QString action() const;

    /**
     * Sets the targeted action of the soap message
     */
    void setAction(const QString& action);

    /**
     * Returns the sender address, default value is Anonymous
     * \see KDSoapAddressingPredefinedAddress enum
     */
    QString sourceEndpoint() const;

    /**
     * Set the origin address of the message
     * In case you do not want to provide it, you might use KDSoapAddressingPredefinedAddress::Anonymous
     */
    void setSourceEndpoint(const QString & sourceEndpoint);

    /**
     * Returns the sender address
     * \see KDSoapAddressingPredefinedAddress enum
     */
    QString replyEndpoint() const;

    /**
     * Sets the reply endpoint the server should reply to
     */
    void setReplyEndpoint(const QString & replyEndpoint);

    /**
     * Returns the fault address, which is the address the server should send the potential fault error
     */
    QString faultEndpoint() const;

    /**
     * Set the fault endpoint address of the message
     */
    void setFaultEndpoint(const QString & faultEndpoint);

    /**
     * Returns the message id
     */
    QString messageID() const;

    /**
     * Set the message id
     */
    void setMessageID(const QString & id);

    /**
     * Returns the QString pair of addresses, it indicates relationship to a prior message to facilitate longer running message exchanges.
     *
     * In case of an standalone / independant message, the predefined value to use is http://www.w3.org/2005/08/addressing/unspecified
     * In case of a reply message, when no value has been set, it is predefined with http://www.w3.org/2005/08/addressing/reply which mean, the second QString
     * of the QPair is the message ID that the message is replying to.
     */
    Relationship relationship() const;

    /**
     * Set the relationship of the message, eg. an previous messageID, no semantic verification is done here.
     */
    void setRelationship(const Relationship relationship);

    /**
     * Returns the custom reference parameters objects, defined within WSDL file
     */
    KDSoapValue referenceParameters() const;

    /**
     * Set the reference parameter, since this value can be anything custom,  it uses a KDSoapValue
     */
    void setReferenceParameters(const KDSoapValue & rp);

    /**
     * Helper function that take the \p address enum to provide the Qstring equivalent
     */
    static QString predefinedAddressToString(KDSoapAddressingPredefinedAddress address);

    /**
     * Write the properties to the soap header, through the QXmlStreamWriter
     */
    void writeMessageAddressingProperties(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, KDSoapValue::Use use, const QString& messageNamespace, bool forceQualified) const;

private:
    QSharedDataPointer<KDSoapMessageAddressingPropertiesData> d;
};


/**
 * Support for debugging KDSoapMessage objects via qDebug() << msg;
 */
KDSOAP_EXPORT QDebug operator <<(QDebug dbg, const KDSoapMessageAddressingProperties &msg);

//Q_DECLARE_METATYPE(KDSoapMessageAddressingProperties)

#endif // KDSOAPMESSAGEADDRESSINGPROPERTIES_H
