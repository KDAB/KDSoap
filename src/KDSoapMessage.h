#ifndef KDSOAPMESSAGE_H
#define KDSOAPMESSAGE_H

#include <QSharedDataPointer>
#include <QVariant>
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

    //QVariant argument(const QString& argumentName) const;

private:
    friend class KDSoapClientInterface;
    friend class KDSoapPendingCall;
    friend QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);
    //KDSoapValueList arguments() const;

    QSharedDataPointer<KDSoapMessageData> d;
};

QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);

#endif // KDSOAPMESSAGE_H
