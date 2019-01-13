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
#include "KDSoapMessageAddressingProperties.h"

#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"

#include <QDebug>
#include <QLatin1String>
#include <QString>
#include <QXmlStreamWriter>

class KDSoapMessageAddressingPropertiesData : public QSharedData
{
public:

    QString destination;    // Provides the address of the intended receiver of this message
    QString action;         // Identifies the semantics implied by this message
    KDSoapEndpointReference sourceEndpoint; // Message origin, could be included to facilitate longer running message exchanges.
    KDSoapEndpointReference replyEndpoint;  // Intended receiver for replies to this message, could be included to facilitate longer running message exchanges.
    KDSoapEndpointReference faultEndpoint;  // Intended receiver for faults related to this message, could be included to facilitate longer running message exchanges.
    QString messageID;      // Unique identifier for this message, may be included to facilitate longer running message exchanges.
    QVector<KDSoapMessageRelationship::Relationship> relationships;   // Indicates relationships to prior messages, could be included to facilitate longer running message exchanges.
    KDSoapValueList referenceParameters; // Equivalent of the reference parameters object from the endpoint reference within WSDL file
    KDSoapValueList metadata; // Holding metadata information
};

KDSoapMessageAddressingProperties::KDSoapMessageAddressingProperties()
    : d(new KDSoapMessageAddressingPropertiesData)
{
}

KDSoapMessageAddressingProperties::KDSoapMessageAddressingProperties(const KDSoapMessageAddressingProperties &other)
    : d(other.d)
{
}

KDSoapMessageAddressingProperties &KDSoapMessageAddressingProperties::operator =(const KDSoapMessageAddressingProperties &other)
{
    d = other.d;
    return *this;
}

QString KDSoapMessageAddressingProperties::destination() const
{
    return d->destination;
}

void KDSoapMessageAddressingProperties::setDestination(const QString &destination)
{
    d->destination = destination;
}

QString KDSoapMessageAddressingProperties::action() const
{
    return d->action;
}

void KDSoapMessageAddressingProperties::setAction(const QString &action)
{
    d->action = action;
}

KDSoapEndpointReference KDSoapMessageAddressingProperties::sourceEndpoint() const
{
    return d->sourceEndpoint;
}

QString KDSoapMessageAddressingProperties::sourceEndpointAddress() const
{
    return d->sourceEndpoint.address();
}

void KDSoapMessageAddressingProperties::setSourceEndpoint(const KDSoapEndpointReference &sourceEndpoint)
{
    d->sourceEndpoint = sourceEndpoint;
}

void KDSoapMessageAddressingProperties::setSourceEndpointAddress(const QString &sourceEndpoint)
{
    d->sourceEndpoint.setAddress(sourceEndpoint);
}

KDSoapEndpointReference KDSoapMessageAddressingProperties::replyEndpoint() const
{
    return d->replyEndpoint;
}

QString KDSoapMessageAddressingProperties::replyEndpointAddress() const
{
    return d->replyEndpoint.address();
}

void KDSoapMessageAddressingProperties::setReplyEndpoint(const KDSoapEndpointReference &replyEndpoint)
{
    d->replyEndpoint = replyEndpoint;
}

void KDSoapMessageAddressingProperties::setReplyEndpointAddress(const QString &replyEndpoint)
{
    d->replyEndpoint.setAddress(replyEndpoint);
}

KDSoapEndpointReference KDSoapMessageAddressingProperties::faultEndpoint() const
{
    return d->faultEndpoint;
}

QString KDSoapMessageAddressingProperties::faultEndpointAddress() const
{
    return d->faultEndpoint.address();
}

void KDSoapMessageAddressingProperties::setFaultEndpoint(const KDSoapEndpointReference &faultEndpoint)
{
    d->faultEndpoint = faultEndpoint;
}

void KDSoapMessageAddressingProperties::setFaultEndpointAddress(const QString &faultEndpoint)
{
    d->faultEndpoint.setAddress(faultEndpoint);
}

QString KDSoapMessageAddressingProperties::messageID() const
{
    return d->messageID;
}

void KDSoapMessageAddressingProperties::setMessageID(const QString &id)
{
    d->messageID = id;
}

QVector<KDSoapMessageRelationship::Relationship> KDSoapMessageAddressingProperties::relationships() const
{
    return d->relationships;
}

void KDSoapMessageAddressingProperties::setRelationships(const QVector<KDSoapMessageRelationship::Relationship> &relationships)
{
    d->relationships = relationships;
}

void KDSoapMessageAddressingProperties::addRelationship(const KDSoapMessageRelationship::Relationship &relationship)
{
    d->relationships.append(relationship);
}

KDSoapValueList KDSoapMessageAddressingProperties::referenceParameters() const
{
    return d->referenceParameters;
}

void KDSoapMessageAddressingProperties::setReferenceParameters(const KDSoapValueList &values)
{
    d->referenceParameters = values;
}

void KDSoapMessageAddressingProperties::addReferenceParameter(const KDSoapValue &oneReferenceParameter)
{
    if (!oneReferenceParameter.isNull()) {
        d->referenceParameters.append(oneReferenceParameter);
    }
}

KDSoapValueList KDSoapMessageAddressingProperties::metadata() const
{
    return d->metadata;
}

void KDSoapMessageAddressingProperties::setMetadata(const KDSoapValueList &metadataList)
{
    d->metadata = metadataList;
}

void KDSoapMessageAddressingProperties::addMetadata(const KDSoapValue &metadata)
{
    if (!metadata.isNull()) {
        d->metadata.append(metadata);
    }
}

KDSoapMessageAddressingProperties::~KDSoapMessageAddressingProperties()
{
}

QString KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::KDSoapAddressingPredefinedAddress address)
{
    switch (address) {
    case Anonymous:
        return QLatin1String("http://www.w3.org/2005/08/addressing/anonymous");
    case None:
        return QLatin1String("http://www.w3.org/2005/08/addressing/none");
    case Reply:
        return QLatin1String("http://www.w3.org/2005/08/addressing/reply");
    case Unspecified:
        return QLatin1String("http://www.w3.org/2005/08/addressing/unspecified");
    default:
        Q_ASSERT(false); // should never happen
        return QString();
    }
}

bool KDSoapMessageAddressingProperties::isWSAddressingNamespace(const QString &namespaceUri)
{
    return namespaceUri == KDSoapNamespaceManager::soapMessageAddressing() ||
            namespaceUri == KDSoapNamespaceManager::soapMessageAddressing200303() ||
            namespaceUri == KDSoapNamespaceManager::soapMessageAddressing200403() ||
            namespaceUri == KDSoapNamespaceManager::soapMessageAddressing200408();
}

static void writeAddressField(QXmlStreamWriter &writer, const QString &address)
{
    writer.writeStartElement(KDSoapNamespaceManager::soapMessageAddressing(), QLatin1String("Address"));
    writer.writeCharacters(address);
    writer.writeEndElement();
}

static void writeKDSoapValueVariant(QXmlStreamWriter &writer, const KDSoapValue &value)
{
    const QVariant valueToWrite = value.value();
    if (valueToWrite.canConvert(QVariant::String)) {
        writer.writeCharacters(valueToWrite.toString());
    } else
        qWarning("Warning: KDSoapMessageAddressingProperties call to writeKDSoapValueVariant could not write the given KDSoapValue "
                 "value because it could not be converted into a QString");
}

static void writeKDSoapValueListHierarchy(KDSoapNamespacePrefixes &namespacePrefixes, QXmlStreamWriter &writer, const KDSoapValueList &values)
{
    const QString addressingNS = KDSoapNamespaceManager::soapMessageAddressing();

    Q_FOREACH (const KDSoapValue &value, values)  {
        const QString topLevelName = value.name();
        writer.writeStartElement(addressingNS, topLevelName);

        if (value.childValues().isEmpty()) {
            writeKDSoapValueVariant(writer, value);
        } else {
            writeKDSoapValueListHierarchy(namespacePrefixes, writer, value.childValues());
        }

        writer.writeEndElement();
    }
}

void KDSoapMessageAddressingProperties::writeMessageAddressingProperties(KDSoapNamespacePrefixes &namespacePrefixes, QXmlStreamWriter &writer, const QString &messageNamespace, bool forceQualified) const
{
    Q_UNUSED(messageNamespace);
    Q_UNUSED(forceQualified);

    if (d->destination == predefinedAddressToString(None)) {
        return;
    }

    if (d->action.isEmpty()) {
        return;
    }

    const QString addressingNS = KDSoapNamespaceManager::soapMessageAddressing();

    if (!d->destination.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("To"));
        writer.writeCharacters(d->destination);
        writer.writeEndElement();
    }

    if (!d->sourceEndpoint.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("From"));
        writeAddressField(writer, d->sourceEndpoint.address());
        writer.writeEndElement();
    }

    if (!d->replyEndpoint.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("ReplyTo"));
        writeAddressField(writer, d->replyEndpoint.address());
        writer.writeEndElement();
    }

    if (!d->faultEndpoint.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("FaultTo"));
        writeAddressField(writer, d->faultEndpoint.address());
        writer.writeEndElement();
    }

    if (!d->action.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("Action"));
        writer.writeCharacters(d->action);
        writer.writeEndElement();
    }

    if (!d->messageID.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("MessageID"));
        writer.writeCharacters(d->messageID);
        writer.writeEndElement();
    }

    foreach (const KDSoapMessageRelationship::Relationship &relationship, d->relationships) {
        if (relationship.uri.isEmpty()) {
            continue;
        }

        writer.writeStartElement(addressingNS, QLatin1String("RelatesTo"));

        if (!relationship.relationshipType.isEmpty()) {
            writer.writeAttribute(QLatin1String("RelationshipType"), relationship.relationshipType);
        }

        writer.writeCharacters(relationship.uri);
        writer.writeEndElement();
    }

    if (!d->referenceParameters.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("ReferenceParameters"));
        writeKDSoapValueListHierarchy(namespacePrefixes, writer, d->referenceParameters);
        writer.writeEndElement();
    }

    if (!d->metadata.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("Metadata"));
        writeKDSoapValueListHierarchy(namespacePrefixes, writer, d->metadata);
        writer.writeEndElement();
    }
}

void KDSoapMessageAddressingProperties::readMessageAddressingProperty(const KDSoapValue &value)
{
    if (value.name() == QLatin1String("Action")) {
        d->action = value.value().toString();
    } else if (value.name() == QLatin1String("MessageID")) {
        d->messageID = value.value().toString();
    } else if (value.name() == QLatin1String("To")) {
        d->destination = value.value().toString();
    } else if (value.name() == QLatin1String("From")) {
        d->sourceEndpoint.setAddress(value.childValues().child(QLatin1String("Address")).value().toString());
    } else if (value.name() == QLatin1String("ReplyTo")) {
        d->replyEndpoint.setAddress(value.childValues().child(QLatin1String("Address")).value().toString());
    } else if (value.name() == QLatin1String("RelatesTo")) {
        KDSoapMessageRelationship::Relationship relationship;
        relationship.uri = (value.value().toString());
        relationship.relationshipType = QLatin1String("http://www.w3.org/2005/08/addressing/reply");
        foreach (KDSoapValue attr, value.childValues().attributes()) {
            if (attr.name() == QLatin1String("RelationshipType")) {
                relationship.relationshipType = attr.value().toString();
            }
        }
        d->relationships.append(relationship);
    } else if (value.name() == QLatin1String("FaultTo")) {
        d->faultEndpoint.setAddress(value.childValues().child(QLatin1String("Address")).value().toString());
    } else if (value.name() == QLatin1String("ReferenceParameters")) {
        d->referenceParameters = value.childValues();
    } else if (value.name() == QLatin1String("Metadata")) {
        d->metadata = value.childValues();
    }
}

QDebug operator <<(QDebug dbg, const KDSoapMessageAddressingProperties &msg)
{
    dbg << msg.action() << msg.destination() << msg.sourceEndpoint().address() << msg.replyEndpoint().address() << msg.faultEndpoint().address() << msg.messageID();

    return dbg;
}
