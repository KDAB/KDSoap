/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_OPERATION_H
#define KWSDL_OPERATION_H

#include <QDomElement>
#include <QList>

#include <wsdl/element.h>
#include <wsdl/fault.h>
#include <wsdl/param.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

/**
 * <operation> as defined inside <portType>.
 * Contains input message, output message, and faults.
 */
class KWSDL_EXPORT Operation : public Element
{
public:
    typedef QList<Operation> List;

    enum OperationType
    {
        OneWayOperation,
        RequestResponseOperation,
        SolicitResponseOperation,
        NotificationOperation
    };

    Operation();
    Operation(const QString &nameSpace);
    ~Operation();

    void setOperationType(OperationType type);
    OperationType operationType() const;

    void setName(const QString &name);
    QString name() const;

    void setInput(const Param &input);
    Param input() const;

    void setOutput(const Param &output);
    Param output() const;

    void setFaults(const Fault::List &faults);
    Fault::List faults() const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    OperationType mType;

    Param mInput;
    Param mOutput;
    Fault::List mFaults;

    QString mName;
};

}

#endif // KWSDL_OPERATION_H
