#ifndef HELLOWORLD_SERVER_H
#define HELLOWORLD_SERVER_H

#include "wsdl_helloworld.h"

class ServerObject : public Hello_ServiceServerBase
{
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)
public:
    ServerObject();
    ~ServerObject();

    QString sayHello(const QString& msg);
};

#endif // HELLOWORLD_SERVER_H
