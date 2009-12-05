#include "KDSoapPendingCallWatcher.h"
#include "KDSoapPendingCallWatcher_p.h"
#include "KDSoapPendingCall_p.h"
#include <QNetworkReply>
#include <QDebug>

KDSoapPendingCallWatcher::KDSoapPendingCallWatcher(const KDSoapPendingCall &call, QObject *parent)
    : QObject(parent), KDSoapPendingCall(call),
    d(new Private(this))
{
    connect(call.d->reply, SIGNAL(finished()), this, SLOT(_kd_slotReplyFinished()));
}

KDSoapPendingCallWatcher::~KDSoapPendingCallWatcher()
{
    delete d;
}

bool KDSoapPendingCallWatcher::isFinished() const
{
    // TODO
    return false;
}

void KDSoapPendingCallWatcher::waitForFinished()
{
    // TODO
}

void KDSoapPendingCallWatcher::Private::_kd_slotReplyFinished()
{
    q->parseReply();
    emit q->finished(q);
}

#include "moc_KDSoapPendingCallWatcher.cpp"
