#include <QCoreApplication>

#include "wsdl_helloworld.h"

#include <iostream>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Hello_Service service;
    service.setEndPoint(QLatin1String("http://localhost:8081"));
    service.setSoapVersion( KDSoapClientInterface::SOAP1_2 );
    const QString reply = service.sayHello(QLatin1String("Hello!"));
    if (!service.lastError().isEmpty())
        std::cerr << qPrintable(service.lastError()) << std::endl;
    else
        std::cout << qPrintable(reply) << std::endl;

    const QString reply2 = service.sayHelloAndGetAnError(QLatin1String("Hello!"));
    if (!service.lastError().isEmpty())
        std::cerr << qPrintable(service.lastError()) << std::endl;
    else
        std::cout << qPrintable(reply2) << std::endl;
    return app.exec();
}
