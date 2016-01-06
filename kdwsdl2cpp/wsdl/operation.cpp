/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "operation.h"

#include <common/messagehandler.h>
#include <common/parsercontext.h>
#include <common/nsmanager.h>
#include <QDebug>

using namespace KWSDL;

Operation::Operation()
    : mType(OneWayOperation)
{
}

Operation::Operation(const QString &nameSpace)
    : Element(nameSpace), mType(OneWayOperation)
{
    mInput.setNameSpace(nameSpace);
    mOutput.setNameSpace(nameSpace);
}

Operation::~Operation()
{
}

void Operation::setOperationType(OperationType type)
{
    mType = type;
}

Operation::OperationType Operation::operationType() const
{
    return mType;
}

void Operation::setName(const QString &name)
{
    mName = name;
}

QString Operation::name() const
{
    return mName;
}

void Operation::setInput(const Param &input)
{
    mInput = input;
}

Param Operation::input() const
{
    return mInput;
}

void Operation::setOutput(const Param &output)
{
    mOutput = output;
}

Param Operation::output() const
{
    return mOutput;
}

void Operation::setFaults(const Fault::List &faults)
{
    mFaults = faults;
}

Fault::List Operation::faults() const
{
    return mFaults;
}

void Operation::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Operation: 'name' required"));
    }

    //http://www.roguewave.com/portals/0/products/hydraexpress/docs/4.6.0/html/rwsfwsdevug/9-2.html
    // input only = OneWayOperation
    // output only = NotificationOperation
    // input+output = RequestResponseOperation
    // output+input = SolicitResponseOperation

    bool hasInput = false;
    bool hasOutput = false;
    bool inputWasFirst = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        const QString tagName = namespaceManager.localName(child);
        if (tagName == QLatin1String("input")) {
            Q_ASSERT(!hasInput);
            if (hasOutput) {
                inputWasFirst = false;
            }
            hasInput = true;
            mInput.loadXML(context, child);
        } else if (tagName == QLatin1String("output")) {
            Q_ASSERT(!hasOutput);
            hasOutput = true;
            mOutput.loadXML(context, child);
        } else if (tagName == QLatin1String("fault")) {
            Fault fault(nameSpace());
            fault.loadXML(context, child);
            mFaults.append(fault);
        } else if (tagName == QLatin1String("documentation")) {
            QString text = child.firstChild().toText().data().trimmed();
            setDocumentation(text);
        } else {
            context->messageHandler()->warning(QString::fromLatin1("Operation: unknown tag %1").arg(child.tagName()));
        }

        child = child.nextSiblingElement();
    }
    Q_ASSERT(hasInput || hasOutput);
    if (hasInput && !hasOutput) {
        mType = OneWayOperation;
    } else if (!hasInput && hasOutput) {
        mType = NotificationOperation;
    } else if (inputWasFirst) {
        mType = RequestResponseOperation;
    } else {
        mType = SolicitResponseOperation;
    }
}

void Operation::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("operation"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("Operation: 'name' required"));
    }

    switch (mType) {
    case OneWayOperation:
        mInput.saveXML(context, QLatin1String("input"), document, element);
        break;
    case SolicitResponseOperation:
        mOutput.saveXML(context, QLatin1String("output"), document, element);
        mInput.saveXML(context, QLatin1String("input"), document, element);
        {
            Fault::List::ConstIterator it(mFaults.begin());
            const Fault::List::ConstIterator endIt(mFaults.end());
            for (; it != endIt; ++it) {
                (*it).saveXML(context, document, element);
            }
        }
        break;
    case NotificationOperation:
        mOutput.saveXML(context, QLatin1String("output)"), document, element);
        break;
    case RequestResponseOperation:
    default:
        mInput.saveXML(context, QLatin1String("input"), document, element);
        mOutput.saveXML(context, QLatin1String("output"), document, element);
        {
            Fault::List::ConstIterator it(mFaults.begin());
            const Fault::List::ConstIterator endIt(mFaults.end());
            for (; it != endIt; ++it) {
                (*it).saveXML(context, document, element);
            }
        }
        break;
    }
}
