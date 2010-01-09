#ifndef KDSOAPPENDINGCALLWATCHER_H
#define KDSOAPPENDINGCALLWATCHER_H

#include <QtCore/QObject>
#include "KDSoapPendingCall.h"

class Q_DECL_EXPORT KDSoapPendingCallWatcher : public QObject, public KDSoapPendingCall
{
    Q_OBJECT
public:
    KDSoapPendingCallWatcher(const KDSoapPendingCall &call, QObject *parent=0);
    ~KDSoapPendingCallWatcher();

#if 0
    bool isFinished() const;
    void waitForFinished();
#endif

Q_SIGNALS:
    void finished(KDSoapPendingCallWatcher *self);

private:
    friend class KDSoapPendingCallPrivate;

    Q_PRIVATE_SLOT(d, void _kd_slotReplyFinished())
    class Private;
    Private* const d;
};

#endif // KDSOAPPENDINGCALLWATCHER_H
