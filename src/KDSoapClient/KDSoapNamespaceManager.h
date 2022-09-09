/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPNAMESPACEMANAGER_H
#define KDSOAPNAMESPACEMANAGER_H

#include "KDSoapGlobal.h"
#include <QtCore/QString>

/**
 * Repository of namespaces
 */
class KDSOAP_EXPORT KDSoapNamespaceManager // krazy:exclude=dpointer
{
public:
    static QString xmlSchema1999();
    static QString xmlSchema2001();
    static QString xmlSchemaInstance1999();
    static QString xmlSchemaInstance2001();
    static QString soapEnvelope();
    static QString soapEnvelope200305();
    static QString soapEncoding();
    static QString soapEncoding200305();
    static QString soapMessageAddressing();
    static QString soapSecurityExtention();
    static QString soapSecurityUtility();
    static QString soapMessageAddressing200303();
    static QString soapMessageAddressing200403();
    static QString soapMessageAddressing200408();

private: // TODO instantiate to handle custom namespaces per clientinterface
    KDSoapNamespaceManager();
};

#endif // KDSOAPNAMESPACEMANAGER_H
