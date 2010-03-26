#ifndef KDSOAPAUTHENTICATION_H
#define KDSOAPAUTHENTICATION_H

#include <QUrl>
class QAuthenticator;
class QNetworkReply;

class Q_DECL_EXPORT KDSoapAuthentication
{
public:
    KDSoapAuthentication();
    KDSoapAuthentication(const KDSoapAuthentication& other);
    ~KDSoapAuthentication();

    void setUser(const QString& user);
    QString user() const;

    void setPassword(const QString& password);
    QString password() const;

    bool hasAuth() const;

    KDSoapAuthentication& operator=(const KDSoapAuthentication& other);

    void handleAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);

private:
    class Private;
    Private * const d;
};

#endif // KDSOAPAUTHENTICATION_H
