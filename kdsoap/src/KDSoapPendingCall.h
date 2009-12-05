#ifndef KDSOAPPENDINGCALL_H
#define KDSOAPPENDINGCALL_H

#include <QtCore/QExplicitlySharedDataPointer>
class QNetworkReply;
class QBuffer;
class KDSoapPendingCallWatcher;

class KDSoapPendingCall
{
public:
    KDSoapPendingCall(const KDSoapPendingCall &other);
    ~KDSoapPendingCall();

    KDSoapPendingCall &operator=(const KDSoapPendingCall &other);

private:
    friend class KDSoapClientInterface;
    KDSoapPendingCall(QNetworkReply* reply, QBuffer* buffer);

    friend class KDSoapPendingCallWatcher;
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif // KDSOAPPENDINGCALL_H
