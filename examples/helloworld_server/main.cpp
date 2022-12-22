/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2011-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include <QCoreApplication>

#include "KDSoapServer.h"
#include "helloworld_serverobject.h"
#include <iostream>

class Server : public KDSoapServer
{
public:
    using KDSoapServer::KDSoapServer;

    QObject *createServerObject() override
    {
        return new HelloWorldServerObject;
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Server server;
    server.setLogLevel(Server::LogEveryCall);
    const bool listening = server.listen(QHostAddress::Any, 8081);
    if (!listening) {
        std::cerr << "Cannot start server: " << qPrintable(server.errorString()) << std::endl;
        return 1;
    } else {
        std::cout << "Listening..." << std::endl;
    }
    return app.exec();
}
