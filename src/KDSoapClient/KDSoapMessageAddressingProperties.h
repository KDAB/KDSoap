/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
#ifndef KDSOAPMESSAGEADDRESSINGPROPERTIES_H
#define KDSOAPMESSAGEADDRESSINGPROPERTIES_H

#include <QtCore/QSharedDataPointer>
#include "KDSoapGlobal.h"

#include "KDSoapEndpointReference.h"
#include "KDSoapValue.h"

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

class KDSoapNamespacePrefixes;
class KDSoapMessageAddressingPropertiesData;

/**
 * Relationship between two soap messages.
 * This class is composed of two QStrings: one represents the type of the relation, the other represents the message ID of the message it refers to.
 *
 * \see https://www.w3.org/TR/ws-addr-core/#msgaddrpropsinfoset
 * \since 1.5
 */
namespace KDSoapMessageRelationship
{

struct Relationship {
    /**
     * Relationship default ctor
     */
    Relationship() {}

    /**
     * Relationship constructor
     * @param URI is supposed to represent a message ID of a previous message you want to make reference to
     * @param type represents the nature of the relation between messages, if none is provided, the following
     * predefined address will be used wsa:Reply (which is dependent on the selected namespace, for example
     * https://www.w3.org/TR/2006/REC-ws-addr-core-20060509)
     */
    Relationship(const QString &URI, const QString &type = QString())
        : uri(URI), relationshipType(type) {}

    QString uri;
    QString relationshipType;
};

} // namespace

/**
 * The KDSoapMessageAddressingProperties class is the abstraction of the
 * WS-Addressing specification. This specification sets up information within the
 * soap envelope header. This class is meant to be filled with the data you want
 * to have in the soap header and associate to a given message using
 * \see KDSoapMessage::setMessageAddressingProperties
 *
 * \see https://www.w3.org/TR/ws-addr-core/#abstractmaps
 * Important: This class does not ensure any kind of validation to the data being passed to it
 * \since 1.5
 */

class KDSOAP_EXPORT KDSoapMessageAddressingProperties
{
public:
    friend class KDSoapMessageWriter;
    friend class KDSoapMessageReader;

    /**
     * This enum contains all the predefined addresses defined by the ws addressing specification
     * It is meant to be used with the predefinedAddress helper function to retrieve the uri as a QString
     * \see predefinedAddressToString
     */
    enum KDSoapAddressingPredefinedAddress {
        None,
        Anonymous,
        Reply,
        Unspecified
    };

    /**
     * This enum contains all the namespaces that can be used to send out WS-Addressing messages.
     * This allows the application to select the WS-Addressing revision to be used.
     * \since 1.9
     * \see setAddressingNamespace
     */
    enum KDSoapAddressingNamespace {
        Addressing200303,
        Addressing200403,
        Addressing200408,
        Addressing200508
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
    void setAction(const QString &action);

    /**
     * Returns the message sender endpoint
     * \see KDSoapEndpointReference
     */
    KDSoapEndpointReference sourceEndpoint() const;

    /**
     * Convenient method, returns directly the source endpoint address
     * \see KDSoapAddressingPredefinedAddress enum
     */
    QString sourceEndpointAddress() const;

    /**
     * Sets the message sender endpoint
     * \see KDSoapEndpointReference
     */
    void setSourceEndpoint(const KDSoapEndpointReference &sourceEndpoint);

    /**
     * Convenient method, sets the message sender address
     */
    void setSourceEndpointAddress(const QString &sourceEndpoint);

    /**
     * Returns the reply endpoint
     * \see KDSoapAddressingPredefinedAddress enum
     */
    KDSoapEndpointReference replyEndpoint() const;

    /**
     * Convenient method, returns the sender endpoint address
     */
    QString replyEndpointAddress() const;

    /**
     * Sets the reply endpoint the server should reply to
     * \see KDSoapEndpointReference
     */
    void setReplyEndpoint(const KDSoapEndpointReference &replyEndpoint);

    /**
     * Convenient method to set directly the reply endpoint address the server should reply to
     */
    void setReplyEndpointAddress(const QString &replyEndpoint);

    /**
     * Returns the fault endpoint, which contains the address the server should send the potential fault error
     */
    KDSoapEndpointReference faultEndpoint() const;

    /**
     * Convenient method that returns the fault endpoint address, which is the address the server should send the potential fault error
     */
    QString faultEndpointAddress() const;

    /**
     * Set the fault endpoint of the message
     * \see KDSoapEndpointReference
     */
    void setFaultEndpoint(const KDSoapEndpointReference &faultEndpoint);

    /**
     * Convenient method to set directly the fault endpoint address of the message
     */
    void setFaultEndpointAddress(const QString &faultEndpoint);

    /**
     * Returns the message id
     */
    QString messageID() const;

    /**
     * Set the message id
     */
    void setMessageID(const QString &id);

    /**
     * Return the relationship of the KDSoapMessageAddressingProperties
     *
     * \see Relationship
     */
    QVector<KDSoapMessageRelationship::Relationship> relationships() const;

    /**
     * Set the relationships of the message, parameter is a QVector of Relationship, the class Relationship carry the relationship type and the message ID of the related message
     *
     * \see Relationship
     *
     */
    void setRelationships(const QVector<KDSoapMessageRelationship::Relationship> &relationships);

    /**
     * Convenient method to add a single Relationship to the message
     *
     * \see Relationship
     *
     */
    void addRelationship(const KDSoapMessageRelationship::Relationship &relationship);

    /**
     * Returns the custom reference parameters objects as a KDSoapValueList
     */
    KDSoapValueList referenceParameters() const;

    /**
     * Set the reference parameters list, since this value can be anything custom, it uses a KDSoapValueList
     */
    void setReferenceParameters(const KDSoapValueList &values);

    /**
     * Add a reference parameter, if not null, to the referenceParameters list
     */
    void addReferenceParameter(const KDSoapValue &oneReferenceParameter);

    /**
     * Returns the metadata of the KDSoapMessageProperties
     */
    KDSoapValueList metadata() const;

    /**
     * Set the metadata field, can be a multi level KDSoapValueList
     */
    void setMetadata(const KDSoapValueList &metadataList);

    /**
     * Add one metadata, if not null, to the list of metadata that will appear within soap header
     */
    void addMetadata(const KDSoapValue &metadata);

    /**
     * Returns the selected WS-Addressing namespace
     * \since 1.9
     */
    KDSoapAddressingNamespace addressingNamespace() const;

    /**
     * Sets the WS-Addressing namespace to be used for sending out messages.This allows the
     * application to select the WS-Addressing revision to be used.
     * \since 1.9
     * \see KDSoapAddressingNamespace
     */
    void setAddressingNamespace(KDSoapAddressingNamespace addressingNamespace);

    /**
     * Helper function that takes the \p address enum and \p addressingNamespace to provide the QString equivalent
     */
    static QString predefinedAddressToString(KDSoapAddressingPredefinedAddress address, KDSoapAddressingNamespace addressingNamespace = Addressing200508);

    /**
     * Helper function that compares \p namespaceUri with the known WS-Addressing namespaces
     */
    static bool isWSAddressingNamespace(const QString& namespaceUri);

    /**
     * Helper function that takes the \p addressingNamespace enum to provide the QString equivalent
     * \since 1.9
     */
    static QString addressingNamespaceToString(KDSoapAddressingNamespace addressingNamespace);

private:
    /**
     * Private method called to write the properties to the soap header, using QXmlStreamWriter
     */
    void writeMessageAddressingProperties(KDSoapNamespacePrefixes &namespacePrefixes, QXmlStreamWriter &writer, const QString &messageNamespace, bool forceQualified) const;

    /**
     * Private method called to read a property from a soap header
     */
    void readMessageAddressingProperty(const KDSoapValue& value);

private:
    QSharedDataPointer<KDSoapMessageAddressingPropertiesData> d;
};

/**
 * Support for debugging KDSoapMessageAddressingProperties object via qDebug() << msg;
 */
KDSOAP_EXPORT QDebug operator <<(QDebug dbg, const KDSoapMessageAddressingProperties &msg);

#endif // KDSOAPMESSAGEADDRESSINGPROPERTIES_H
