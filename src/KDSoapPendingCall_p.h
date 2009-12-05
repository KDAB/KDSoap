#ifndef KDSOAPPENDINGCALL_P_H
#define KDSOAPPENDINGCALL_P_H

#include <QSharedData>
#include <QBuffer>

class QNetworkReply;

class KDSoapPendingCall::Private : public QSharedData
{
public:
    Private(QNetworkReply* r, QBuffer* b)
        : reply(r), buffer(b)
    {
    }
    ~Private()
    {
        delete buffer;
    }

    QNetworkReply* reply;
    QBuffer* buffer;
};

#endif // KDSOAPPENDINGCALL_P_H
