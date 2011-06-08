#ifndef HELLOWORLD_SERVER_H
#define HELLOWORLD_SERVER_H

#include "wsdl_helloworld.h"

class ServerObject : public Hello_ServiceServerBase
{
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)
public:
    ServerObject() : Hello_ServiceServerBase() {
    }

    ~ServerObject() {
    }

    QString sayHello(const QString& msg)
    {
    if (msg.isEmpty()) {
            setFault(QLatin1String("Client.Data"), QLatin1String("Empty message"),
                     QLatin1String("ServerObject"), tr("You must say something.") ;
            return QString();
        }
        return tr("I'm helloworld_server and you said: %1").arg(msg);
    }

    QString sayHelloAndGetAnError(const QString&) {
        setFault(QLatin1String("Client.Data"), QLatin1String("Some error"),
                 QLatin1String("ServerObject"), tr("This method always returns an error."));
        return QString();
    }
};

#endif // HELLOWORLD_SERVER_H
