/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2011 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef HELLOWORLD_SERVER_H
#define HELLOWORLD_SERVER_H

#include "KDSoapServerObjectInterface.h"
#include "wsdl_helloworld.h"

// clazy:excludeall=ctor-missing-parent-argument
class HelloWorldServerObject : public Hello_ServiceServerBase
{
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)
public:
    HelloWorldServerObject();
    ~HelloWorldServerObject();

    QString sayHello(const QString &msg) override;
};

#endif // HELLOWORLD_SERVER_H
