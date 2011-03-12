#ifndef KDSOAPPENDINGCALL_P_H
#define KDSOAPPENDINGCALL_P_H

#include <QSharedData>
#include <QBuffer>
#include <QXmlStreamReader>
#include "KDSoapMessage.h"
#if QT_VERSION >= 0x040600
#include <QWeakPointer>
#else
#include <QPointer>
#endif

class QNetworkReply;
class KDSoapValue;

class KDSoapPendingCall::Private : public QSharedData
{
public:
    Private(QNetworkReply* r, QBuffer* b)
        : reply(r), buffer(b), parsed(false)
    {
    }
    ~Private();

    void parseReply();
    KDSoapValue parseReplyElement(QXmlStreamReader& reader);

    // Can be deleted under us if the KDSoapClientInterface (and its QNetworkAccessManager)
    // are deleted before the KDSoapPendingCall.
#if QT_VERSION >= 0x040600
    QWeakPointer<QNetworkReply> reply;
#else
    QPointer<QNetworkReply> reply;
#endif
    QBuffer* buffer;
    KDSoapMessage replyMessage;
    KDSoapHeaders replyHeaders;
    bool parsed;
};

#endif // KDSOAPPENDINGCALL_P_H
