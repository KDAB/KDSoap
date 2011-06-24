#include <QtCore/QCoreApplication>
#include "WSDL/integrationTestServer.h"
#include "integrationTestService.h"
#include <QThreadPool>
#include <KDSoap>
#include "KDSoapServer.h"
#include <QThread>
#include "KDSoapThreadPool.h"
#include <QHostAddress>



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug()<<"Start";
    IntegrationTestServer server;
    IntegrationTestServer * pServer ;
    KDSoapThreadPool * threadPool=new KDSoapThreadPool();

    server.setThreadPool(threadPool);
    server.setPath("/path");

    qDebug()<<"StartServer";
    //server.setThreadPool(QThreadPool ::globalInstance());
    if (server.listen(QHostAddress("127.0.0.1"),6949))
        pServer = &server;
    //connect(&server, SIGNAL(releaseSemaphore()), this, SLOT(slotReleaseSemaphore()), Qt::DirectConnection);

    qDebug()<<"Server is lisening: "<<server.endPoint();

    return a.exec();
}
