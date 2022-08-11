/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2011-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include <QCoreApplication>

#include "KDSoapServer.h"
#include "helloworld_serverobject.h"
#include <iostream>

HelloWorldServerObject::HelloWorldServerObject()
    : Hello_ServiceServerBase()
{
}

HelloWorldServerObject::~HelloWorldServerObject()
{
}

QString HelloWorldServerObject::sayHello(const QString &msg)
{
    if (msg.isEmpty()) {
        setFault(QLatin1String("Client.Data"), QLatin1String("Empty message"), QLatin1String("HelloWorldServerObject"), tr("You must say something."));
        return QString();
    }
    return tr("I'm helloworld_server and you said: %1").arg(msg);
}

