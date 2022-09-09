/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_COMPILER_H
#define KWSDL_COMPILER_H

#include <QDomElement>
#include <QObject>

namespace KWSDL {

class Compiler : public QObject
{
    Q_OBJECT

public:
    Compiler();

public slots:
    void run();

private slots:
    void download();
    void parse(const QDomElement &);
};

}

#endif
