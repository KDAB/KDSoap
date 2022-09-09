/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "wsdl_keep_unused_types.h"

#include <QTest>
#include <QDebug>

class KeepUnusedTypesArgumentKDWSDL2CPP : public QObject
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
