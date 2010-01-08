#ifndef KDSOAPMESSAGE_H
#define KDSOAPMESSAGE_H

#include <QSharedDataPointer>
#include <QVariant>
#include "KDSoapValue.h"
class QString;
class KDSoapMessageData;
class QXmlStreamWriter;

class KDSoapMessage
{
public:
    KDSoapMessage();
    ~KDSoapMessage();

    KDSoapMessage(const KDSoapMessage& other);
    KDSoapMessage &operator=(const KDSoapMessage &other);

    void addArgument(const QString& argumentName, const QVariant& argumentValue);

    KDSoapValueList arguments() const;

    bool isFault() const;
    QString faultAsString() const;

private:
    friend class KDSoapClientInterface;
    friend class KDSoapPendingCall;
    friend QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);
    void setFault(bool fault);

    QSharedDataPointer<KDSoapMessageData> d;
};

QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);

#endif // KDSOAPMESSAGE_H
