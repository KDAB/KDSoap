/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_ELEMENT_H
#define KWSDL_ELEMENT_H

#include <QString>

#include <kode_export.h>

namespace KWSDL {

class KWSDL_EXPORT Element
{
public:
    Element();
    Element(const QString &nameSpace);
    ~Element();

    void setNameSpace(const QString &nameSpace);
    QString nameSpace() const;

    void setDocumentation(const QString &documentation);
    QString documentation() const;

private:
    QString mNameSpace;
    QString mDocumentation;
};

}

#endif // KWSDL_ELEMENT_H
