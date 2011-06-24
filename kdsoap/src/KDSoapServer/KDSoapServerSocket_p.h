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

private Q_SLOTS:
    void slotReadyRead();

private:
    void makeCall(const KDSoapMessage& requestMsg, KDSoapMessage& replyMsg,
                  const KDSoapHeaders& requestHeaders, KDSoapHeaders& responseHeaders,
                  const QByteArray& soapAction);
    void handleError(KDSoapMessage& replyMsg, const char* errorCode, const QString& error);

    KDSoapSocketList* m_owner;
    QObject* m_serverObject;
    bool m_doDebug;
    QByteArray m_requestBuffer;
};

#endif // KDSOAPSERVERSOCKET_P_H
