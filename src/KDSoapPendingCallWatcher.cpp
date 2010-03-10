#include "KDSoapPendingCallWatcher.h"
#include "KDSoapPendingCallWatcher_p.h"
#include "KDSoapPendingCall_p.h"
#include <QNetworkReply>
#include <QDebug>

KDSoapPendingCallWatcher::KDSoapPendingCallWatcher(const KDSoapPendingCall &call, QObject *parent)
    : QObject(parent), KDSoapPendingCall(call),
    d(new Private(this))
{
    connect(call.d->reply.data(), SIGNAL(finished()), this, SLOT(_kd_slotReplyFinished()));
}

KDSoapPendingCallWatcher::~KDSoapPendingCallWatcher()
{
    delete d;
}

#if 0
void KDSoapPendingCallWatcher::waitForFinished()
{
    // TODO?
}
#endif

void KDSoapPendingCallWatcher::Private::_kd_slotReplyFinished()
{
    //q->parseReply(); // done on demand
    emit q->finished(q);
}

#include "moc_KDSoapPendingCallWatcher.cpp"
