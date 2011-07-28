#ifndef KDSOAPSERVERSOCKET_P_H
#define KDSOAPSERVERSOCKET_P_H

#include <QtGlobal>

#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#else
#include <QTcpSocket>
#endif

#include <QMap>
class QObject;
class KDSoapSocketList;
class KDSoapServerObjectInterface;
class KDSoapMessage;
class KDSoapHeaders;

class KDSoapServerSocket
#ifndef QT_NO_OPENSSL
        : public QSslSocket
#else
        : public QTcpSocket
#endif
{
    Q_OBJECT
public:
    KDSoapServerSocket(KDSoapSocketList* owner, QObject* serverObject);
    ~KDSoapServerSocket();

    void sendDelayedReply(KDSoapServerObjectInterface* serverObjectInterface, const KDSoapMessage& replyMsg);
    void sendReply(KDSoapServerObjectInterface* serverObjectInterface, const KDSoapMessage& replyMsg);

private Q_SLOTS:
    void slotReadyRead();

private:
    void makeCall(KDSoapServerObjectInterface* serverObjectInterface,
                  const KDSoapMessage& requestMsg, KDSoapMessage& replyMsg,
                  const KDSoapHeaders& requestHeaders,
                  const QByteArray& soapAction);
    void handleError(KDSoapMessage& replyMsg, const char* errorCode, const QString& error);
    void setSocketEnabled(bool enabled);

    KDSoapSocketList* m_owner;
    QObject* m_serverObject;
    bool m_doDebug;
    bool m_socketEnabled;
    QByteArray m_requestBuffer;

    // Data for the current call (stored here for delayed replies)
    QString m_messageNamespace;
    QString m_method;
};

#endif // KDSOAPSERVERSOCKET_P_H
