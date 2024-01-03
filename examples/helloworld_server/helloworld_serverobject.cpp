/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2011 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
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
