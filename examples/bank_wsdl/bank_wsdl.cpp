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

    std::cout << "\"" << qPrintable(response.details().bezeichnung()) << "\" in "
              << qPrintable(response.details().ort()) << std::endl;

    return 0;
}
