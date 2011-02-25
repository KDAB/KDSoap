#ifndef KDSOAPSERVERSOCKET_P_H
#define KDSOAPSERVERSOCKET_P_H

#include <QTcpSocket>
#include <QMap>
class QObject;
class KDSoapSocketList;
class KDSoapMessage;

class KDSoapServerSocket : public QTcpSocket
{
    Q_OBJECT
public:
    KDSoapServerSocket(KDSoapSocketList* owner, QObject* serverObject);
    ~KDSoapServerSocket();

private Q_SLOTS:
    void slotReadyRead();

private:
    void makeCall(const KDSoapMessage& requestMsg, KDSoapMessage& replyMsg);
    void handleError(KDSoapMessage& replyMsg, const char* errorCode, const QString& error);

    KDSoapSocketList* m_owner;
    QObject* m_serverObject;
    bool m_doDebug;

    QByteArray m_receivedData;
    QByteArray m_receivedHttpHeaders;
    QMap<QByteArray, QByteArray> m_httpHeaders;
};

#endif // KDSOAPSERVERSOCKET_P_H
