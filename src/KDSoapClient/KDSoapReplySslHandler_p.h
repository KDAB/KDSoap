#ifndef KDSOAPREPLYSSLHANDLER_P_H
#define KDSOAPREPLYSSLHANDLER_P_H

#include <QObject>
QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

#ifndef QT_NO_OPENSSL
#include <QSslError>
class KDSoapSslHandler;

class KDSoapReplySslHandler : public QObject
{
    Q_OBJECT
public:
    explicit KDSoapReplySslHandler(QNetworkReply *reply, KDSoapSslHandler *handler);

private Q_SLOTS:
    void slotReplySslErrors(const QList<QSslError> &errors);

private:
    KDSoapSslHandler *m_handler;
};

#endif
#endif // KDSOAPREPLYSSLHANDLER_P_H
