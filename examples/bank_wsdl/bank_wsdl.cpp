/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include <QCoreApplication>

#include <iostream>

#include "wsdl_BLZService.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const QString bankCode = QString::fromLatin1("20130600");
    std::cout << "Looking up the bank with code " << qPrintable(bankCode) << "..." << std::endl;

    BLZService::BLZServiceSOAP11Binding service;
    TNS__GetBankType getBankType;
    getBankType.setBlz(bankCode);
    TNS__GetBankResponseType response = service.getBank(getBankType);

    std::cout << "\"" << qPrintable(response.details().bezeichnung()) << "\" in " << qPrintable(response.details().ort()) << std::endl;

    return 0;
}
