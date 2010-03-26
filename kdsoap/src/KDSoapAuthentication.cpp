#include "KDSoapAuthentication.h"
#include <QNetworkReply>
#include <QDebug>
#include <QAuthenticator>

class KDSoapAuthentication::Private
{
public:
    QString user;
    QString password;
};

KDSoapAuthentication::KDSoapAuthentication()
    : d(new Private)
{
}

KDSoapAuthentication::KDSoapAuthentication(const KDSoapAuthentication& other)
    : d(new Private)
{
    *d = *other.d;
}

KDSoapAuthentication &KDSoapAuthentication::operator=(const KDSoapAuthentication &other)
{
    *d = *other.d;
    return *this;
}

KDSoapAuthentication::~KDSoapAuthentication()
{
    delete d;
}

void KDSoapAuthentication::setUser(const QString &user)
{
    d->user = user;
}

void KDSoapAuthentication::setPassword(const QString &password)
{
    d->password = password;
}

QString KDSoapAuthentication::user() const
{
    return d->user;
}

QString KDSoapAuthentication::password() const
{
    return d->password;
}

bool KDSoapAuthentication::hasAuth() const
{
    return !d->user.isEmpty() || !d->password.isEmpty();
}

void KDSoapAuthentication::handleAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    //qDebug() << "handleAuthenticationRequired" << reply << reply->url() << "realm=" << authenticator->realm();
    // Only proceed if
    // 1) we have some authentication to offer
    // 2) we didn't try once already (unittest: BuiltinHttpTest::testAsyncCallRefusedAuth)
    if (hasAuth() && !reply->property("authAdded").toBool()) {
        authenticator->setUser(d->user);
        authenticator->setPassword(d->password);
        reply->setProperty("authAdded", true);
    } else {
        // protected... reply->setError(QNetworkReply::AuthenticationRequiredError, QString());
        reply->abort();
    }
}
