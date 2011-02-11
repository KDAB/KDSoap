#include "KDSoapSocketPool_p.h"
#include <QTcpSocket>

const bool doDebug = true;

KDSoapSocketPool::KDSoapSocketPool(QObject* parent)
    : QObject(parent)
{
}

void KDSoapSocketPool::handleIncomingConnection(int socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(disconnected()),
            socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
    //m_sockets.append(socket); // would only be useful for count()
}

void KDSoapSocketPool::slotReadyRead()
{
    QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());
    qDebug() << "slotReadyRead!";
    const QByteArray request = clientSocket->readAll();
    if (doDebug) {
        qDebug() << "KDSoapSocketPool: request:" << request;
    }
}

#include "moc_KDSoapSocketPool_p.cpp"
