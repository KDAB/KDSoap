#include <QCoreApplication>

#include "KDSoapServer.h"
#include "helloworld_server.h"
#include <iostream>

ServerObject::ServerObject()
    : Hello_ServiceServerBase()
{
}

ServerObject::~ServerObject() {
}

QString ServerObject::sayHello(const QString& msg)
{
    if (msg.isEmpty()) {
        setFault(QLatin1String("Client.Data"), QLatin1String("Empty message"),
                 QLatin1String("ServerObject"), tr("You must say something."));
        return QString();
    }
    return tr("I'm helloworld_server and you said: %1").arg(msg);
}

class Server : public KDSoapServer {
public:
    QObject* createServerObject() override {
        return new ServerObject;
    }
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    Server server;
    server.setLogLevel(Server::LogEveryCall);
    const bool listening = server.listen(QHostAddress::Any, 8081);
    if ( !listening ) {
        std::cerr << "Cannot start server: " << qPrintable(server.errorString()) << std::endl;
        return 1;
    } else {
        std::cout << "Listening..." << std::endl;
    }
    return app.exec();
}
