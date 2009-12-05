#ifndef KDSOAPPENDINGCALL_H
#define KDSOAPPENDINGCALL_H

#include <QtCore/QExplicitlySharedDataPointer>
#include "KDSoapMessage.h"
class QNetworkReply;
class QBuffer;
class KDSoapPendingCallWatcher;

class KDSoapPendingCall
{
public:
    KDSoapPendingCall(const KDSoapPendingCall &other);
    ~KDSoapPendingCall();

    KDSoapPendingCall &operator=(const KDSoapPendingCall &other);

    QVariant returnValue() const;
    KDSoapMessage returnArguments() const;

private:
    friend class KDSoapClientInterface;
    KDSoapPendingCall(QNetworkReply* reply, QBuffer* buffer);

    friend class KDSoapPendingCallWatcher;
    void parseReply();

    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif // KDSOAPPENDINGCALL_H
