/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_IMPORT_H
#define KWSDL_IMPORT_H

#include <QDomElement>
#include <QList>
#include <QUrl>

#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Import : public Element
{
public:
    typedef QList<Import> List;

    Import();
    Import(const QString &nameSpace);
    ~Import();

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

    void setImportNamespace(const QString &nameSpace);
    QString importNamespace() const;

    void setLocation(const QUrl &location);
    QUrl location() const;

private:
    QString mImportNamespace;
    QUrl mLocation;
};

}

#endif // KWSDL_IMPORT_H
