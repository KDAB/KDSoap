#include "KDSoapReplySslHandler_p.h"
#include "KDSoapSslHandler.h"
#include <QNetworkReply>

#ifndef QT_NO_OPENSSL

KDSoapReplySslHandler::KDSoapReplySslHandler(QNetworkReply *reply, KDSoapSslHandler *handler) :
    QObject(reply), m_handler(handler)
{
    Q_ASSERT(reply);
    Q_ASSERT(handler);
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotReplySslErrors(QList<QSslError>)));
}

void KDSoapReplySslHandler::slotReplySslErrors(const QList<QSslError> &errors)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(parent());
    Q_ASSERT(reply);
    m_handler->handleSslErrors(reply, errors);
}

#endif
