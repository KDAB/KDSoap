#ifndef KDSOAPSERVERSOCKET_P_H
#define KDSOAPSERVERSOCKET_P_H

#include <QTcpSocket>
#include <QMap>
class QObject;
class KDSoapSocketList;

class KDSoapServerSocket : public QTcpSocket
{
    Q_OBJECT
public:
    KDSoapServerSocket(KDSoapSocketList* owner, QObject* serverObject);
    ~KDSoapServerSocket();

private Q_SLOTS:
    void slotReadyRead();

private:
    KDSoapSocketList* m_owner;
    QObject* m_serverObject;

    QByteArray m_receivedData;
    QByteArray m_receivedHeaders;
    QMap<QByteArray, QByteArray> m_headers;
};

#endif // KDSOAPSERVERSOCKET_P_H
