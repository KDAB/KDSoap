/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_NAMEMAPPER_H
#define KWSDL_NAMEMAPPER_H

#include <QStringList>

namespace KWSDL {

class NameMapper
{
public:
    NameMapper();

    QString escape(const QString &name) const;
    QString unescape(const QString &name) const;

private:
    QStringList mKeyWords;
};

}

#endif
