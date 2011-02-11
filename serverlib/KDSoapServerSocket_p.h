#ifndef KDSOAPSERVERSOCKET_P_H
#define KDSOAPSERVERSOCKET_P_H

#include <QTcpSocket>
class KDSoapServerObject;
class KDSoapSocketList;

class KDSoapServerSocket : public QTcpSocket
{
    Q_OBJECT
public:
    KDSoapServerSocket(KDSoapSocketList* owner, KDSoapServerObject* serverObject);
    ~KDSoapServerSocket();

private Q_SLOTS:
    void slotReadyRead();

private:
    KDSoapSocketList* m_owner;
    KDSoapServerObject* m_serverObject;
};

#endif // KDSOAPSERVERSOCKET_P_H
