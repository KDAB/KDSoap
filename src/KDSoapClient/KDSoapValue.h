/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
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

#ifndef QT_NO_STL
# include <algorithm>
#endif

// Qt-4.7 errors on QVariant::fromValue<signed char>(), but later versions support it.
#if QT_VERSION < 0x040800
Q_DECLARE_METATYPE(signed char)
#endif

class KDSoapValueList;
class KDSoapNamespacePrefixes;
QT_BEGIN_NAMESPACE
class QXmlStreamWriter;
QT_END_NAMESPACE

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
     * \param valueVariant the value of the argument
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
    KDSoapValue& operator=(const KDSoapValue& other) {
        if ( this != &other ) {
            KDSoapValue copy( other );
            swap( copy );
        }
        return *this;
    }

    /**
     * Swaps the contents of \a other with the contents of \c this. Never throws.
     */
    void swap( KDSoapValue& other ) {
#if QT_VERSION < 0x040600
        qSwap( reinterpret_cast<Private*&>(d), reinterpret_cast<Private*&>(other.d) );
#else
        d.swap( other.d );
#endif
    }

    /**
     * Returns true if this KDSoapValue was created with the default constructor.
     */
    bool isNull() const;

    /**
     * Returns the name of the argument, as passed to the constructor.
     */
    QString name() const;

    /**
     * Returns the namespace of this argument, if it differs from the "message namespace".
     */
    QString namespaceUri() const;

    /**
     * Sets the namespace of this argument, if it differs from the "message namespace".
     */
    void setNamespaceUri(const QString& ns);

    /**
     * Returns the value of the argument.
     */
    QVariant value() const;

    /**
     * Sets the \p value of the argument.
     */
    void setValue(const QVariant& value);

    /**
     * Whether the element should be qualified in the XML. See setQualified()
     *
     * \since 1.2
     */
    bool isQualified() const;

    /**
     * Set whether the element should be qualified. Qualified means, that
     * locally declared elements and attributes are qualified by a namespace,
     * using an explicit prefix. Default is unqualified.
     *
     * Note that this property does not propagate to child values. It needs to be set
     * for each child value (they could come from another schema which doesn't specify
     * the same value for qualified).
     *
     * \param qualified Whether to qualify the element, or not.
     * \since 1.2
     */
    void setQualified(bool qualified);

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
     * Compares two KDSoapValues.
     */
    bool operator!=(const KDSoapValue& other) const;

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

    /**
     * Defines the way the message should be serialized.
     * See the "use" attribute for soap:body, in the WSDL file.
     */
    enum Use
    {
      LiteralUse, ///< data is serialized according to a given schema, no \c xsi:type attributes are written out
      EncodedUse  ///< each message part references an abstract type using the \c xsi:type attribute
    };

    QByteArray toXml(Use use = LiteralUse, const QString& messageNamespace = QString()) const;

private:
    // To catch mistakes
    KDSoapValue(QString, QString, QString);

    friend class KDSoapMessageWriter;
    void writeElement(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, KDSoapValue::Use use, const QString& messageNamespace, bool forceQualified) const;
    void writeElementContents(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, KDSoapValue::Use use, const QString& messageNamespace) const;
    void writeChildren(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, KDSoapValue::Use use, const QString& messageNamespace, bool forceQualified) const;

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_TYPEINFO( KDSoapValue, Q_MOVABLE_TYPE );

KDSOAP_EXPORT QDebug operator <<(QDebug dbg, const KDSoapValue &value);

KDSOAP_EXPORT uint qHash( const KDSoapValue& value );
inline void qSwap( KDSoapValue & lhs, KDSoapValue & rhs ) { lhs.swap( rhs ); }

#ifndef QT_NO_STL
namespace std {
    template <> inline void swap<KDSoapValue>( KDSoapValue & lhs, KDSoapValue & rhs ) {
        lhs.swap( rhs );
    }
}
#endif

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
     * \param argumentValue the value of the argument
     * \param typeNameSpace namespace of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     * \param typeName localname of the type of this value; this is only useful if using KDSoapMessage::EncodedUse
     *
     * Note that this doesn't allow to call KDSoapValue::setQualified() or KDSoapValue::setNamespaceUri() on the value.
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
     *
     * If no such argument can be found, returns a null KDSoapValue.
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
