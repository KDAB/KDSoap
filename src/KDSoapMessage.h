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

    /**
     * Adds an argument to the message.
     *
     * Equivalent to arguments().append(KDSoapValue(argumentName, argumentValue));
     */
    void addArgument(const QString& argumentName, const QVariant& argumentValue);

    /**
     * Returns the arguments for the message.
     * The list can be modified, in order to modify the message.
     */
    KDSoapValueList& arguments();

    /**
     * Returns the arguments for the message.
     * The list is readonly; useful for inspecting a response.
     */
    const KDSoapValueList& arguments() const;

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
