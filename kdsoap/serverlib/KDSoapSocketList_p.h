#ifndef KDSOAPSOCKETLIST_P_H
#define KDSOAPSOCKETLIST_P_H

#include <QSet>
class QTcpSocket;
class KDSoapServer;
class KDSoapServerObject;
class KDSoapServerSocket;

class KDSoapSocketList
{
public:
    KDSoapSocketList(KDSoapServer* server);

    KDSoapServerSocket* handleIncomingConnection(int socketDescriptor);

    void socketDeleted(KDSoapServerSocket* socket);

    int socketCount() const;

private:
    KDSoapServerObject* m_serverObject;
    QSet<KDSoapServerSocket *> m_sockets;
};

#endif // KDSOAPSOCKETLIST_P_H
