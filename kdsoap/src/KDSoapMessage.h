#ifndef KDSOAPMESSAGE_H
#define KDSOAPMESSAGE_H

#include <QSharedDataPointer>
#include <QVariant>
#include "KDSoapValue.h"
class QString;
class KDSoapMessageData;

/**
 * The KDSoapMessage class represents one message sent or received via SOAP.
 */
class KDSOAP_EXPORT KDSoapMessage : public KDSoapValue
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
     * Constructs a copy of the object given by \p other.
     */
    KDSoapMessage(const KDSoapMessage& other);
    /**
     * Copies the contents of the object given by \p other.
     */
    KDSoapMessage &operator=(const KDSoapMessage &other);

    /**
     * Defines the way the message should be serialized.
     * See the "use" attribute for soap:body, in the WSDL file.
     */
    enum Use
    {
      LiteralUse, ///< data is serialized according to a given schema, no \c xsi:type attributes are written out
      EncodedUse  ///< each message part references an abstract type using the \c xsi:type attribute
    };

    /**
     * Define the way the message should be serialized.
     * The default value is #LiteralUse.
     */
    void setUse(Use use);
    /**
     * Returns the value passed to setUse().
     */
    Use use() const;

    /**
     * Adds an argument to the message.
     *
     * \param argumentName the argument name (which corresponds to the element or attribute name in the XML)
     * \param argumentValue this QVariant can hold either a simple value, or a KDSoapValueList of child values.
     *          (the KDSoapValueList support is mostly for the convenience of the \c kdwsdl2cpp generated code)
     * \param typeNameSpace namespace of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     * \param typeName localname of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     *
     * Equivalent to
     * \code
     * arguments().addArgument(argumentName, argumentValue [, typeNameSpace, typeName] );
     * \endcode
     */
    void addArgument(const QString& argumentName, const QVariant& argumentValue, const QString& typeNameSpace = QString(), const QString& typeName = QString());

    /**
     * Adds a complex-type argument to the message.
     *
     * \param argumentName the argument name (which corresponds to the element or attribute name in the XML)
     * \param argumentValueList KDSoapValueList of child values.
     * \param typeNameSpace namespace of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     * \param typeName localname of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     *
     * Equivalent to
     * \code
     * arguments().append(KDSoapValue(argumentName, argumentValueList [, typeNameSpace, typeName] ));
     * \endcode
     */
    void addArgument(const QString& argumentName, const KDSoapValueList& argumentValueList, const QString& typeNameSpace = QString(), const QString& typeName = QString());

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
     * \return \c true if this message is a "fault" message.
     * A fault message is the message returned by a SOAP server when an error occurred.
     */
    bool isFault() const;

    /**
     * \return the fault message as a string that can be shown to the user.
     */
    QString faultAsString() const;

private:
    void setFault(bool fault);
    void parseSoapXml(const QByteArray& data);
    friend class KDSoapClientInterface;
    friend class KDSoapPendingCall;
    friend QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);

    QSharedDataPointer<KDSoapMessageData> d;
};

/**
 * Set of headers that can be provided when making a SOAP call.
 * \see KDSoapClientInterface
 */
class KDSoapHeaders : public QList<KDSoapMessage>
{
};

/**
 * Support for debugging KDSoapMessage objects via qDebug() << msg;
 */
QDebug operator<<(QDebug dbg, const KDSoapMessage &msg);

#endif // KDSOAPMESSAGE_H
