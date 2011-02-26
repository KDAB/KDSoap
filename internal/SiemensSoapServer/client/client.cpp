#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "wsdl_IPGW_V1.0.h"
#include <QtTest/QtTest>
#include <QDebug>

class Test : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCall()
    {
        AmisTS_IPGateway gateway;
        gateway.setEndPoint(QString::fromLatin1("http://localhost:4245"));
        gateway.setSoapVersion(KDSoapClientInterface::SOAP1_2);

        AMIS__IPGatewayDataRequest request;
        request.setEquipInformationBlock(QByteArray("Test_EquipInfoBlock"));
        request.setTelegram(QByteArray("Test_telegram"));
        AMIS__IPGatewayDataResponse response = gateway.iPGatewayData(request); // sync call
        if (!gateway.lastError().isEmpty()) {
            qDebug() << "ERROR:" << gateway.lastError();
            QVERIFY(false);
        } else {
            qDebug() << response.telegram();
        }
    }

};

QTEST_MAIN(Test)

#include "client.moc"


