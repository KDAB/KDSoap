#ifndef KDSOAPSOCKETPOOL_P_H
#define KDSOAPSOCKETPOOL_P_H

#include <QObject>
class QTcpSocket;

class KDSoapSocketPool : public QObject
{
    Q_OBJECT
public:
    KDSoapSocketPool(QObject* parent);

    void handleIncomingConnection(int socketDescriptor);

private Q_SLOTS:
    void slotReadyRead();

private:
    //QSet<QTcpSocket *> m_sockets;
};

#endif // KDSOAPSOCKETPOOL_P_H
