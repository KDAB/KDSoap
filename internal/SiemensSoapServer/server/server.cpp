#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapServer.h"
#include "KDSoapThreadPool.h"
#include "KDSoapServerObjectInterface.h"
#include "wsdl_IPGW_V1.0.h"
#include <QDebug>
#include <QCoreApplication>

class IPGatewayServerObject : public AmisTS_IPGatewayServerBase
{
public:
    AMIS__IPGatewayDataResponse iPGatewayData(const AMIS__IPGatewayDataRequest& parameters) {
        qDebug() << "iPGatewayData() called! Input=" << parameters.telegram();
        AMIS__IPGatewayDataResponse response;
        response.setTelegram(QByteArray("This is the response to telegram ") + parameters.telegram());
        return response;
    }
};

class IPGatewayServer : public KDSoapServer
{
    Q_OBJECT
public:
    IPGatewayServer() : KDSoapServer() {}
    virtual QObject* createServerObject() { return new IPGatewayServerObject; }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    IPGatewayServer server;
    if (!server.listen(QHostAddress::LocalHost, 4245)) {
        qWarning("Could not listen on port 4245, maybe another server is already running?");
        return 1;
    }

    qDebug() << "Server listening on" << server.endPoint();

    return app.exec();
}

// #include "swsdl_IPGW_V1.0.cpp"

#include "server.moc"
