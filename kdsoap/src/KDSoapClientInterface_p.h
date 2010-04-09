#ifndef KDSOAPCLIENTINTERFACE_P_H
#define KDSOAPCLIENTINTERFACE_P_H

#include <QNetworkAccessManager>
#include <QXmlStreamWriter>

#include "KDSoapClientInterface.h"
#include "KDSoapClientThread_p.h"
#include "KDSoapAuthentication.h"
class QBuffer;
class KDSoapMessage;
class KDSoapNamespacePrefixes;

class KDSoapClientInterface::Private : public QObject
{
    Q_OBJECT
public:
    Private();

    // Warning: this accessManager is only used by asyncCall and callNoReply.
    // For blocking calls, the thread has its own accessManager.
    QNetworkAccessManager m_accessManager;
    QString m_endPoint;
    QString m_messageNamespace;
    KDSoapClientThread m_thread;
    KDSoapAuthentication m_authentication;

    QNetworkRequest prepareRequest(const QString &method, const QString& action);
    QBuffer* prepareRequestBuffer(const QString& method, const KDSoapMessage& message);
    void writeArguments(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValueList& args, KDSoapMessage::Use use);

private Q_SLOTS:
    void _kd_slotAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);
};

#endif // KDSOAPCLIENTINTERFACE_P_H
