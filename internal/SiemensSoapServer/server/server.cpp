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

class IPGatewayServerObject : public QObject, public KDSoapServerObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)
public:
    IPGatewayServerObject() : QObject(), KDSoapServerObjectInterface() {
    }

    virtual void processRequest(const KDSoapMessage &request, KDSoapMessage &response)
    {
        // This code will be generated in the future
        const QByteArray method = request.name().toLatin1();
        qDebug() << method;
        if (method == "IPGatewayData") {
            //if (values.isEmpty()) {
            //    response.setFault(true);
            //    response.addArgument(QLatin1String("faultcode"), QLatin1String("Server.RequiredArgumentMissing"));
            //    return;
            //}
            AMIS__IPGatewayDataRequest parameters;
            parameters.deserialize(request);
            AMIS__IPGatewayDataResponse ret = iPGatewayData(parameters);
            if (!hasFault()) {
                response.childValues() += ret.serialize(QString()).childValues();
            }
        }
    }

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
    server.listen(QHostAddress::LocalHost, 4245);

    qDebug() << "Server listening on" << server.endPoint();

    return app.exec();
}

#include "server.moc"
