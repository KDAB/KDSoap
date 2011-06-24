#ifndef INTEGRATIONTESTSERVICE_H
#define INTEGRATIONTESTSERVICE_H
#include "WSDL/integrationTestServer.h"
#include <KDSoap>
#include <QThread>
#include "KDSoapServer.h"

class IntegrationTestService : public IntegrationTestServerBase
{
    Q_OBJECT
public:
    IntegrationTestService();
public:
    int numberTransfer(int numberTransferRequest);
    QString stringTransfer(const QString &stringTransferRequest);
    TNS__NumberList listOfNumbers(const TNS__NumberList &listOfNumbersRequest);
    TNS__StringList listOfStrings(const TNS__StringList &listOfStringsRequest);
    TNS__ContainerType containerTransfer(const TNS__ContainerType &containerTransferRequest);

};

class IntegrationTestServer : public KDSoapServer
{
    Q_OBJECT
public:
    IntegrationTestServer() : KDSoapServer() {}
    virtual QObject* createServerObject() { return new IntegrationTestService; }

Q_SIGNALS:
    void releaseSemaphore();

public Q_SLOTS:
    void quit() { thread()->quit(); }
#if 0
    void suspend() { KDSoapServer::suspend(); qDebug() << "server suspended"; emit releaseSemaphore(); }
    void resume() { KDSoapServer::resume(); emit releaseSemaphore(); }
#endif
};
#endif // INTEGRATIONTESTSERVICE_H
