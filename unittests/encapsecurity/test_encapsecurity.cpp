/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapValue.h"
#include "httpserver_p.h"
#include "wsdl_authstateless.h"
#include <QDebug>
#include <QEventLoop>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

class EncapSecurityTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void minimalTest()
    {
        EncapAuthenticationStatelessPortImplService service;
        __AuthnRequestType request;
        __ContextResponseType response;

        Q_UNUSED(service);
        Q_UNUSED(request);
        Q_UNUSED(response);
    }
};

QTEST_MAIN(EncapSecurityTest)

#include "test_encapsecurity.moc"
