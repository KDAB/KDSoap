#ifndef KDSOAPMESSAGE_H
#define KDSOAPMESSAGE_H

#include <QSharedDataPointer>
#include <QVariant>
#include "KDSoapValue.h"
class QString;
class KDSoapMessageData;
class QXmlStreamWriter;

/**
 * The KDSoapMessage class represents one message sent or received via SOAP.
 */
class Q_DECL_EXPORT KDSoapMessage
{
public:
    /**
     * Constructs an empty KDSoapMessage object.
     */
    KDSoapMessage();
    /**
     * Destructs the KDSoapMessage object.
     */
    ~KDSoapMessage();

    /**
     * Constructs a copy of the object given by @p other.
     */
    KDSoapMessage(const KDSoapMessage& other);
    /**
     * Copies the contents of the object given by @p other.
     */
    KDSoapMessage &operator=(const KDSoapMessage &other);

    enum Use
    {
      LiteralUse, ///< data is serialized according to a given schema, no xsi:type attributes are written out
      EncodedUse  ///< each message part references an abstract type using the xsi:type attribute
    };

    /**
     * Define the way the message should be serialized
     */
    void setUse(Use use);
    Use use() const;

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

    /**
     * @return true if this message is a "fault" message.
     * A fault message is the message returned by a SOAP server when an error occurred.
     */
    bool isFault() const;
    /**
     * @return the fault message as a string that can be shown to the user.
     */
    QString faultAsString() const;

private:
    friend class KDSoapClientInterface;
    friend class KDSoapPendingCall;
    friend QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);
    void setFault(bool fault);

    QSharedDataPointer<KDSoapMessageData> d;
};

/**
 * Set of headers that can be provided when making a SOAP call.
 * @see KDSoapClientInterface
 */
class KDSoapHeaders : public QList<KDSoapMessage>
{
};

/**
 * Support for debugging KDSoapMessage objects via qDebug() << msg;
 */
QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);

#endif // KDSOAPMESSAGE_H
