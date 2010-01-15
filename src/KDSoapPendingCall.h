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

    KDSoapMessage returnMessage() const;

    /// Helper method for the simple case where a single argument is returned:
    /// Returns the value of that single argument.
    QVariant returnValue() const;

private:
    friend class KDSoapClientInterface;
    friend class KDSoapThreadTask;
    KDSoapPendingCall(QNetworkReply* reply, QBuffer* buffer);

    friend class KDSoapPendingCallWatcher;
    void parseReply();

    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif // KDSOAPPENDINGCALL_H
