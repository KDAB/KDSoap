#ifndef KDSOAPVALUE_H
#define KDSOAPVALUE_H

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QSet>
#include <QtCore/QVector>
#include <QtCore/QSharedDataPointer>
#include "KDSoapGlobal.h"

class KDSoapValueList;

/**
 * KDSoapValue represents a value in a SOAP argument list.
 * It is composed of the argument name, and actual value as a QVariant.
 *
 * Optionally, the type name can be set (will become the \c xsi:type attribute,
 * which gives type information at runtime).
 *
 * In terms of the actual XML being sent or received, this represents one XML element
 * or one XML attribute.
 * childValues() contains the child XML elements of this XML element.
 */
class KDSOAP_EXPORT KDSoapValue
{
public:
    /**
     * Constructs an empty KDSoapValue.
     * This usually indicates an error, e.g. when KDSoapValueList::child() doesn't find the child.
     */
    KDSoapValue();
    /**
     * Destructor.
     */
    ~KDSoapValue();

    /**
     * Constructs a value from a QVariant.
     *
     * \param name the argument name (which corresponds to the element or attribute name in the XML)
     * \param valueVariant this QVariant can hold either a simple value, or a KDSoapValueList of child values.
     *          (the KDSoapValueList support is mostly for the convenience of the kdwsdl2cpp generated code)
     * \param typeNameSpace namespace of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     * \param typeName localname of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     */
    KDSoapValue(const QString& name, const QVariant& valueVariant, const QString& typeNameSpace = QString(), const QString& typeName = QString());
    /**
     * Constructs a "complex" value with child values
     *
     * \param name the argument name (which corresponds to the element or attribute name in the XML)
     * \param childValues the child elements and the attributes of this value.
     * \param typeNameSpace namespace of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     * \param typeName localname of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     */
    KDSoapValue(const QString& name, const KDSoapValueList& childValues, const QString& typeNameSpace = QString(), const QString& typeName = QString());

    /**
     * Copy constructor
     */
    KDSoapValue(const KDSoapValue& other);
    /**
     * Assignment operator
     */
    KDSoapValue& operator=(const KDSoapValue& other);

    /**
     * Returns the name of the argument, as passed to the constructor.
     */
    QString name() const;

    /**
     * Returns the value of the argument.
     */
    QVariant value() const;

    /**
     * Sets the \p value of the argument.
     */
    void setValue(const QVariant& value);

    /**
     * Returns the list of child values (elements and attributes).
     * The list is a reference, and can therefore be modified.
     */
    KDSoapValueList& childValues() const;

    /**
     * Compares two KDSoapValues.
     */
    bool operator==(const KDSoapValue& other) const;

    /**
     * Sets the type information for this KDSoapValue, so that it can be sent
     * in the \c xsi:type attribute.
     * This is only useful if using KDSoapMessage::EncodedUse.
     *
     * For instance
     * \code
     * setType("http://www.w3.org/2001/XMLSchema-instance", "string")
     * \endcode
     * will send \c xsi:type="xsd:string" in the message XML.
     *
     * \param nameSpace namespace of the type of this value
     * \param type localname of the type of this value
     */
    void setType(const QString& nameSpace, const QString& type);
    /**
     * Returns the namespace of the type.
     * Example: "http://www.w3.org/2001/XMLSchema-instance".
     */
    QString typeNs() const;
    /**
     * Returns the localname of the type.
     * Example: "string".
     */
    QString type() const;

private:
    // To catch mistakes
    KDSoapValue(QString, QString, QString);
    class Private;
    QSharedDataPointer<Private> d;
};

KDSOAP_EXPORT QDebug operator <<(QDebug dbg, const KDSoapValue &value);

KDSOAP_EXPORT uint qHash( const KDSoapValue& value );

/**
 * KDSoapValueList represents a list of arguments passed to a SOAP message.
 *
 * In other words, it corresponds to a list of XML elements in a SOAP
 * message. It also supports XML attributes.
 */
class KDSOAP_EXPORT KDSoapValueList : public QList<KDSoapValue>
{
public:
    /**
     * Convenience method for adding an argument to the list.
     *
     * \param argumentName the argument name (which corresponds to the element or attribute name in the XML)
     * \param argumentValue this QVariant can hold either a simple value, or a KDSoapValueList of child values.
     *          (the KDSoapValueList support is mostly for the convenience of the \c kdwsdl2cpp generated code)
     * \param typeNameSpace namespace of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     * \param typeName localname of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     *
     * Equivalent to
     * \code
     * append(KDSoapValue(argumentName, argumentValue [, typeNameSpace, typeName] ));
     * \endcode
     */
    void addArgument(const QString& argumentName, const QVariant& argumentValue, const QString& typeNameSpace = QString(), const QString& typeName = QString());

    /**
     * Convenience method for extracting a child argument by \p name.
     * If multiple arguments have the same name, the first match is returned.
     * This method mostly makes sense for the case where only one argument uses \p name.
     */
    KDSoapValue child(const QString& name) const;

    /**
     * Sets the type of the elements in this array.
     *
     * This is sent as the \c soap-enc:arrayType attribute in the XML.
     *
     * \param nameSpace namespace of the type of this value
     * \param type localname of the type of this value
     */
    void setArrayType(const QString& nameSpace, const QString& type);
    /**
     * Return the namespace of the type of elements in the array.
     */
    QString arrayTypeNs() const;
    /**
     * Return the localname of the type of elements in the array.
     */
    QString arrayType() const;

    /**
     * Returns the list of attributes. Just like the QList which is KDSoapValueList contains
     * the child elements for a parent XML element; the attributes QList is the attributes
     * for that same parent XML element. Note that this is rarely used in SOAP messages though.
     *
     * The returned list can be modified, e.g. to append new attributes.
     */
    QList<KDSoapValue>& attributes() { return m_attributes; }
    /**
     * Read-only getter for the attributes.
     */
    const QList<KDSoapValue>& attributes() const { return m_attributes; }

private:
    QPair<QString, QString> m_arrayType;
    QList<KDSoapValue> m_attributes;

    QVariant d; // for extensions
};

typedef QListIterator<KDSoapValue> KDSoapValueListIterator;

//Q_DECLARE_METATYPE(KDSoapValueList)

#endif // KDSOAPVALUE_H
