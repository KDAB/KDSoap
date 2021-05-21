/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include <QCoreApplication>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"

#include <iostream>

#include "wsdl_BLZService.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const QString blz = QString::fromLatin1("20130600");
    std::cout << "Looking up the bank with BLZ code " << qPrintable(blz) << "..." << std::endl;

    BLZService::BLZServiceSOAP11Binding service;
    TNS__GetBankType getBankType;
    getBankType.setBlz(blz);
    TNS__GetBankResponseType response = service.getBank(getBankType);

    std::cout << "\"" << qPrintable(response.details().bezeichnung()) << "\" in " << qPrintable(response.details().ort()) << std::endl;

    return 0;
}
