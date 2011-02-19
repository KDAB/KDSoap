#ifndef KDSOAPSERVERTHREAD_P_H
#define KDSOAPSERVERTHREAD_P_H

#include <QThread>
#include <QHash>
class KDSoapServer;
class KDSoapSocketList;

class KDSoapServerThread : public QThread
{
    Q_OBJECT
public:
    explicit KDSoapServerThread(QObject *parent = 0);
    ~KDSoapServerThread();

    void handleIncomingConnection(int socketDescriptor, KDSoapServer* server);

    int socketCount() const;

private:
    KDSoapSocketList* socketListForServer(KDSoapServer* server);
    typedef QHash<KDSoapServer*, KDSoapSocketList*> SocketLists;
    SocketLists m_socketLists;
};

#endif // KDSOAPSERVERTHREAD_P_H
