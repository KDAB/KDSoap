#ifndef KDSOAPSOCKETLIST_P_H
#define KDSOAPSOCKETLIST_P_H

#include <QSet>
class QTcpSocket;
class KDSoapServer;
class QObject;
class KDSoapServerSocket;

class KDSoapSocketList
{
public:
    KDSoapSocketList(KDSoapServer* server);
    ~KDSoapSocketList();

    KDSoapServerSocket* handleIncomingConnection(int socketDescriptor);

    void socketDeleted(KDSoapServerSocket* socket);

    int socketCount() const;

    KDSoapServer* server() const { return m_server; }

private:
    KDSoapServer* m_server;
    QObject* m_serverObject;
    QSet<KDSoapServerSocket *> m_sockets;
};

#endif // KDSOAPSOCKETLIST_P_H
