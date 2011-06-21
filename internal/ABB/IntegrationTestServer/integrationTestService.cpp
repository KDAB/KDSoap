#include "integrationTestService.h"
#include "ServerTestCommons.h"
#include "WSDL/integrationTestServer.h"
IntegrationTestService::IntegrationTestService()
{
}

int IntegrationTestService::numberTransfer(int numberTransferRequest)
{
    int r=numberTransferRequest*10;
    if(DEBUG_LEVEL>1)
        qDebug()<<"someone asked me about "<<numberTransferRequest<< " ... I said "<<r;
    return r;
}

QString IntegrationTestService::stringTransfer(const QString &stringTransferRequest)
{
	QString r=stringTransferRequest +" Return";
	if(DEBUG_LEVEL>1)
        qDebug()<<"he said "<<stringTransferRequest<< " ... I said "<<r;
    return r;
}

TNS__NumberList IntegrationTestService::listOfNumbers(const TNS__NumberList &listOfNumbersRequest)
{
    if(DEBUG_LEVEL>1)
    {
        qDebug()<<"List of numbers containing "<< listOfNumbersRequest.number().count()<<"items";
    }
    TNS__NumberList nl(listOfNumbersRequest);
    return nl;
}


TNS__StringList IntegrationTestService::listOfStrings(const TNS__StringList &listOfStringsRequest)
{
    TNS__StringList nl(listOfStringsRequest);
    return nl;
}

TNS__ContainerType IntegrationTestService::containerTransfer(const TNS__ContainerType &containerTransferRequest)
{
    TNS__ContainerType nc;
    QDateTime dt= containerTransferRequest.dateTime().addDays(1);
    nc.setDateTime(dt);
    nc.setString("Hello World");
    TNS__NumberList nl;
    QList<int> list;
    list<<10<<20<<30;
    nl.setNumber(list);;
    nc.setNumberList(nl);
    return nc;
}
