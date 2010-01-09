#include "soapresponder.h"
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapPendingCallWatcher.h>

class SoapResponder::Private
{
public:
    Private(SoapResponder* qq) : m_clientInterface(NULL), q(qq) {}
    ~Private() { delete m_clientInterface; }

    void _kd_slotMethod1Finished(KDSoapPendingCallWatcher* watcher);

    KDSoapClientInterface* clientInterface();

    KDSoapClientInterface* m_clientInterface;
    KDSoapMessage m_lastReply;
    SoapResponder* q;
};

SoapResponder::SoapResponder(QObject *parent) :
    QObject(parent), d(new Private(this))
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

QString SoapResponder::lastError() const
{
    if (d->m_lastReply.isFault())
        return d->m_lastReply.faultAsString();
    return QString();
}

QString SoapResponder::Method1(const QString &bstrParam1, const QString &bstrParam2)
{
    const QString action = QString::fromLatin1("http://www.SoapClient.com/SoapObject");
    KDSoapMessage message;
    message.addArgument(QLatin1String("bstrParam1"), bstrParam1);
    message.addArgument(QLatin1String("bstrParam2"), bstrParam2);
    d->m_lastReply = d->clientInterface()->call(QLatin1String("Method1"), message, action);
    if (d->m_lastReply.isFault())
        return QString();
    return d->m_lastReply.arguments().first().value().toString();
}

void SoapResponder::asyncMethod1(const QString &bstrParam1, const QString &bstrParam2)
{
    const QString action = QString::fromLatin1("http://www.SoapClient.com/SoapObject");
    KDSoapMessage message;
    message.addArgument(QLatin1String("bstrParam1"), bstrParam1);
    message.addArgument(QLatin1String("bstrParam2"), bstrParam2);
    KDSoapPendingCall pendingCall = d->clientInterface()->asyncCall(QLatin1String("Method1"), message, action);
    KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
    connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
            this, SLOT(_kd_slotMethod1Finished(KDSoapPendingCallWatcher*)));
}

void SoapResponder::Private::_kd_slotMethod1Finished(KDSoapPendingCallWatcher *watcher)
{
    KDSoapMessage reply = watcher->returnArguments();
    if (reply.isFault()) {
        emit q->Method1Error(reply);
    } else {
        emit q->Method1Done(reply.arguments().first().value().toString());
    }
}

#include "moc_soapresponder.cpp"
