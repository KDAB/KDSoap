#ifndef KDSOAPAUTHENTICATION_H
#define KDSOAPAUTHENTICATION_H

#include "KDSoapGlobal.h"
#include <QUrl>
class QAuthenticator;
class QNetworkReply;

/**
 * KDSoapAuthentication provides an authentication object.
 * Currently it only supports authentication based on user/password,
 * but its design makes it extensible to other forms of authentication.
 *
 * \see KDSoapClientInterface::setAuthentication()
 */
class KDSOAPCLIENT_EXPORT KDSoapAuthentication
{
public:
    /**
     * Constructs an empty authentication object.
     */
    KDSoapAuthentication();
    /**
     * Constructs a copy of \p other.
     */
    KDSoapAuthentication(const KDSoapAuthentication& other);
    /**
     * Destructs the object
     */
    ~KDSoapAuthentication();

    /**
     * Sets the \p user used for authentication
     */
    void setUser(const QString& user);
    /**
     * \return the user used for authentication
     */
    QString user() const;

    /**
     * Sets the \p password used for authentication
     */
    void setPassword(const QString& password);
    /**
     * \return the password used for authentication
     */
    QString password() const;

    /**
     * \return \c true if authentication was defined, or
     * \c false if this object is only a default-constructed KDSoapAuthentication().
     */
    bool hasAuth() const;

    /**
     * Assigns the contents of \p other to this authenticator.
     */
    KDSoapAuthentication& operator=(const KDSoapAuthentication& other);

    /**
     * \internal
     */
    void handleAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);

private:
    class Private;
    Private * const d;
};

#endif // KDSOAPAUTHENTICATION_H
