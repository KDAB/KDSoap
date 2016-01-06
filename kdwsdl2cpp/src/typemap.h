/*
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2010 David Faure <dfaure@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KWSDL_TYPEMAP_H
#define KWSDL_TYPEMAP_H

#include <QStringList>

#include <common/qname.h>
#include <schema/types.h>

static const QString XMLSchemaURI(QLatin1String("http://www.w3.org/2001/XMLSchema"));
static const QString XMLSchemaInstanceURI(QLatin1String("http://www.w3.org/2001/XMLSchema-instance"));

class NSManager;

namespace KWSDL
{

class TypeMap
{
public:

    TypeMap();
    ~TypeMap();

    void setNSManager(NSManager *manager);

    /**
     * Returns true if @p typeName refers to a "plain old datatype", like int or char.
     * Example: @p typeName is "xsd:nonPositiveInteger".
     */
    bool isBasicType(const QName &typeName) const;
    /**
     * Returns true if @p typeName refers to a builtin type,
     * i.e. a type we always know about, from XML schema.
     * Example: @p typeName is "xsd:string".
     */
    bool isBuiltinType(const QName &typeName) const;
    bool isComplexType(const QName &typeName) const;

    /**
     * Returns true if @p typeName refers to a complex type with derived classes
     * i.e. storing it as a value would lead to truncation. A shared pointer has to be used instead.
     */
    bool isPolymorphic(const QName &typeName) const;

    /**
     * Returns true if @p typeName is the special type "any" or "anyType".
     */
    bool isTypeAny(const QName &typeName) const;

    QString localType(const QName &typeName) const;
    // unused QString baseType( const QName &typeName ) const;
    QStringList headers(const QName &typeName) const;
    QStringList forwardDeclarations(const QName &typeName) const;
    QStringList headerIncludes(const QName &typeName) const;
    //QString localNameSpace( const QName &typeName ) const;

    //QStringList headersForElement( const QName &typeName ) const;
    QStringList forwardDeclarationsForElement(const QName &typeName) const;

    /// Convenience methods:
    /// Returns the local type for the given type or element (either one or the other is set)
    QString localType(const QName &typeName, const QName &elementName) const;
    /// Returns the local type as an "input" parameter, for the given type or element
    /// (either one or the other is set)
    /// For instance "const QString&" for typeName = "xsd:string",
    /// and "const MyElement&" for elementName = "MyElement".
    QString localInputType(const QName &typeName, const QName &elementName) const;

    /**
     * Returns true if @p typeName (or @p elementName, only one is set) refers to a complex type,
     * i.e. one with multiple named values, rather than just one value.
     */
    bool isComplexType(const QName &typeName, const QName &elementName) const;
    /**
     * Returns true if @p typeName (or @p elementName, only one is set) refers to a builtin type,
     * i.e. a type we always know about, from XML schema.
     * Example: @p typeName is "xsd:string".
     */
    bool isBuiltinType(const QName &typeName, const QName &elementName) const;

    /**
     * Returns true if @p typeName (or @p elementName, only one is set) refers to a complex type with derived classes
     * i.e. storing it as a value would lead to truncation
     */
    bool isPolymorphic(const QName &typeName, const QName &elementName) const;

    /**
     * Return C++ code for converting the variant in "var" into the right type.
     */
    QString deserializeBuiltin(const QName &typeName, const QName &elementName, const QString &var, const QString &qtTypeName) const;
    QString serializeBuiltin(const QName &typeName, const QName &elementName, const QString &var, const QString &qtTypeName) const;

    QString localTypeForAttribute(const QName &typeName) const;
    QStringList headersForAttribute(const QName &typeName) const;
    QStringList forwardDeclarationsForAttribute(const QName &typeName) const;
    QString localNameSpaceForAttribute(const QName &typeName) const;

    void addSchemaTypes(const XSD::Types &types, const QString &ns);

    void dump() const;

private:
    void addBuiltinType(const char *typeName, const char *localType);
    QString localTypeForElement(const QName &elementName) const;

    /// If this element derives from a simple type (e.g. xml:hexBinary, xml:base64Binary, xml:dateTime), return that.
    QName baseTypeForElement(const QName &elementName) const;

    class Entry
    {
    public:
        Entry() : basicType(false), builtinType(false), complexType(false), isPolymorphic(false) {}
        bool basicType;   // POD (int, bool, etc.)
        bool builtinType; // types defined in xmlschema
        bool complexType;
        bool isPolymorphic; // has derived classes -> store as shared pointer
        QString nameSpace;
        QString typeName;
        QString localType;
        QName baseType;
        QStringList headers;
        QStringList forwardDeclarations;
        QStringList headerIncludes;

        QString dumpBools() const;
    };

    QList<Entry>::ConstIterator typeEntry(const QName &typeName) const;
    QList<Entry>::ConstIterator elementEntry(const QName &typeName) const;

    QList<Entry> mTypeMap;
    QList<Entry> mElementMap;
    QList<Entry> mAttributeMap;

    NSManager *mNSManager;
};

}

#endif
