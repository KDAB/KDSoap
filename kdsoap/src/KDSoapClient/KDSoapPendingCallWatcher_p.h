#ifndef KDSOAPPENDINGCALLWATCHER_P_H
#define KDSOAPPENDINGCALLWATCHER_P_H

class KDSoapPendingCallWatcher::Private
{
public:
    Private(KDSoapPendingCallWatcher* qq)
        : q(qq)
    {}
    void _kd_slotReplyFinished();

    KDSoapPendingCallWatcher* q;
};

#endif // KDSOAPPENDINGCALLWATCHER_P_H
