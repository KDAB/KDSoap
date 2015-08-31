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
#include "KDSoapMessageAddressingProperties.h"

#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"

#include <QDebug>
#include <QLatin1String>
#include <QPair>
#include <QString>
#include <QXmlStreamWriter>

class KDSoapMessageAddressingPropertiesData : public QSharedData
{
public:
    KDSoapMessageAddressingPropertiesData(){}

    QString destination;    // Provides the address of the intended receiver of this message
    QString action;         // Identifies the semantics implied by this message
    QString sourceEndpoint; // Message origin, could be included to facilitate longer running message exchanges.
    QString replyEndpoint;  // Intended receiver for replies to this message, could be included to facilitate longer running message exchanges.
    QString faultEndpoint;  // Intended receiver for faults related to this message, could be included to facilitate longer running message exchanges.
    QString messageID;      // Unique identifier for this message, may be included to facilitate longer running message exchanges.
    Relationship relationship;   // Indicates relationship to a prior message, could be included to facilitate longer running message exchanges.
    KDSoapValue referenceParameters; // Equivalent of the reference parameters object from the endpoint reference within WSDL file
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

QString KDSoapMessageAddressingProperties::sourceEndpoint() const
{
    return d->sourceEndpoint;
}

void KDSoapMessageAddressingProperties::setSourceEndpoint(const QString &sourceEndpoint)
{
    d->sourceEndpoint = sourceEndpoint;
}

QString KDSoapMessageAddressingProperties::replyEndpoint() const
{
    return d->replyEndpoint;
}

void KDSoapMessageAddressingProperties::setReplyEndpoint(const QString &replyEndpoint)
{
    d->replyEndpoint = replyEndpoint;
}

QString KDSoapMessageAddressingProperties::faultEndpoint() const
{
    return d->faultEndpoint;
}

void KDSoapMessageAddressingProperties::setFaultEndpoint(const QString &faultEndpoint)
{
    d->faultEndpoint = faultEndpoint;
}

QString KDSoapMessageAddressingProperties::messageID() const
{
    return d->messageID;
}

void KDSoapMessageAddressingProperties::setMessageID(const QString &id)
{
    d->messageID = id;
}

Relationship KDSoapMessageAddressingProperties::relationship() const
{
    return d->relationship;
}

void KDSoapMessageAddressingProperties::setRelationship(const Relationship relationship)
{
    d->relationship = relationship;
}

KDSoapValue KDSoapMessageAddressingProperties::referenceParameters() const
{
    return d->referenceParameters;
}

void KDSoapMessageAddressingProperties::setReferenceParameters(const KDSoapValue &rp)
{
    d->referenceParameters = rp;
}

KDSoapMessageAddressingProperties::~KDSoapMessageAddressingProperties()
{
}

QString KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::KDSoapAddressingPredefinedAddress address)
{
    switch (address) {
        case Anonymous:
            return QLatin1String("http://www.w3.org/2005/08/addressing/anonymous");
            break;
        case None:
            return QLatin1String("http://www.w3.org/2005/08/addressing/none");
            break;
        case Reply:
            return QLatin1String("http://www.w3.org/2005/08/addressing/reply");
            break;
        case Unspecified:
            return QLatin1String("http://www.w3.org/2005/08/addressing/unspecified");
            break;
        default:
            Q_ASSERT(false); // should never happen
            return QLatin1String("");
            break;
    }
}

static void writeAddressField(QXmlStreamWriter &writer, const QString& address)
{
    writer.writeStartElement(KDSoapNamespaceManager::soapMessageAddressing(), QLatin1String("Address"));
    writer.writeCharacters(address);
    writer.writeEndElement();
}

void KDSoapMessageAddressingProperties::writeMessageAddressingProperties(KDSoapNamespacePrefixes &namespacePrefixes, QXmlStreamWriter &writer, KDSoapValue::Use use, const QString &messageNamespace, bool forceQualified) const
{
    if (d->destination == predefinedAddressToString(None) || d->destination.isEmpty() )
        return; // we discard the message

    if (d->action.isEmpty())
        return;

    const QString addressingNS = KDSoapNamespaceManager::soapMessageAddressing();

    writer.writeStartElement(addressingNS, QLatin1String("To"));
    writer.writeCharacters(d->destination);
    writer.writeEndElement();

    writer.writeStartElement(addressingNS, QLatin1String("From"));
    writeAddressField(writer, d->sourceEndpoint);
    writer.writeEndElement();

    if (!d->replyEndpoint.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("ReplyTo"));
        writeAddressField(writer, d->replyEndpoint);
        writer.writeEndElement();
    }

    if (!d->faultEndpoint.isEmpty()) {
        writer.writeStartElement(addressingNS, QLatin1String("FaultTo"));
        writeAddressField(writer, d->faultEndpoint);
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

    if (!d->relationship.first.isEmpty() || !d->relationship.second.isEmpty() ) {
        writer.writeStartElement(addressingNS, QLatin1String("RelatesTo"));
        // TODO: support RelatedTo serilization
        writer.writeEndElement();
    }

    if (!d->referenceParameters.isNull()) {
        writer.writeStartElement(addressingNS, QLatin1String("ReferenceParameters"));
        // TODO: support referenceParameters serilization
        writer.writeEndElement();
    }
}

QDebug operator <<(QDebug dbg, const KDSoapMessageAddressingProperties &msg)
{
    dbg << msg.action() << msg.destination() << msg.sourceEndpoint() << msg.replyEndpoint() << msg.faultEndpoint() << msg.messageID();

    return dbg;
}

