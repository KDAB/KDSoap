/****************************************************************************
** Copyright (C) 2010-2018 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDSoapClientInterface.h"
#include "wsdl_keep_unused_types.h"

#include <QtTest/QtTest>
#include <QDebug>

class KeepUnusedTypesArgumentKDWSDL2CPP: public QObject
{
    Q_OBJECT

private slots:

    void testKeepUnusedTypesArgument()
    {
        // Unused type within WSDL, if this compiles, then -keep-unused-types works since
        // otherwise cleanupUnusedTypes function would have removed it from the cpp generation
        TNS__UnusedElement element;
        Q_UNUSED(element);
    }
};

QTEST_MAIN(KeepUnusedTypesArgumentKDWSDL2CPP)

#include "keep_unused_types.moc"

