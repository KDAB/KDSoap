#include "KDSoapServerSocket_p.h"
#include "KDSoapSocketList_p.h"

KDSoapServerSocket::KDSoapServerSocket(KDSoapSocketList* owner, KDSoapServerObject* serverObject)
    : QTcpSocket(),
      m_owner(owner),
      m_serverObject(serverObject)
{
    connect(this, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
}

KDSoapServerSocket::~KDSoapServerSocket()
{
    m_owner->socketDeleted(this);
}

void KDSoapServerSocket::slotReadyRead()
{
    qDebug() << "slotReadyRead!";
    const QByteArray request = this->readAll();
    if (true /*doDebug*/) {
        qDebug() << "KDSoapServerSocket: request:" << request;
    }
    // TODO call method on m_serverObject
}

#include "moc_KDSoapServerSocket_p.cpp"
