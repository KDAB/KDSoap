/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
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
