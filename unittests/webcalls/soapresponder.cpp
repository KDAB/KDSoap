#include "soapresponder.h"
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>

class SoapResponder::Private
{
public:
    Private() : m_clientInterface(NULL) {}
    ~Private() { delete m_clientInterface; }

    KDSoapClientInterface* clientInterface();

    KDSoapClientInterface* m_clientInterface;
    KDSoapMessage m_lastReply;
};

SoapResponder::SoapResponder(QObject *parent) :
    QObject(parent), d(new Private)
{
}

SoapResponder::~SoapResponder()
{
    delete d;
}

KDSoapClientInterface *SoapResponder::Private::clientInterface()
{
    if (!m_clientInterface) {
        const QString endPoint = QString::fromLatin1("http://soapclient.com/xml/soapresponder.wsdl");
        const QString messageNamespace = QString::fromLatin1("http://www.SoapClient.com/xml/SoapResponder.xsd");
        m_clientInterface = new KDSoapClientInterface(endPoint, messageNamespace);
    }
    return m_clientInterface;
}

QString SoapResponder::Method1(const QString &bstrParam1, const QString &bstrParam2)
{
    const QString action = QString::fromLatin1("http://www.SoapClient.com/SoapObject");
    KDSoapMessage message;
    message.addArgument(QLatin1String("bstrParam1"), bstrParam1);
    message.addArgument(QLatin1String("bstrParam2"), bstrParam2);
    d->m_lastReply = d->clientInterface()->call("Method1", message, action);
    if (d->m_lastReply.isFault())
        return QString();
    return d->m_lastReply.arguments().first().value().toString();
}

QString SoapResponder::lastError() const
{
    return d->m_lastReply.faultAsString();
}
