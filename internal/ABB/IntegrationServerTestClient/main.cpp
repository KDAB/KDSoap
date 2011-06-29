#include <QtCore>
#include "WSDL/integrationTestClient.h"
#define SERVICEURL "http://127.0.0.1:6949/path"
#include <QtDebug>
#include <QDateTime>

#define DEBUG_LEVEL 1
#define TEST_REPETITIONS 1000


//I hate that QThread::sleep(uint) is protected
class QWrapperThread : public QThread{
 public:
 static void Sleep(unsigned long msecs) { QThread::msleep(msecs); }
 };

 inline void qSleep(uint t) { QWrapperThread::Sleep(t); }


void testNumbersList(int numbersCount,int repetitions, IntegrationTest * client)
{
    qDebug()<<"Test: Large Number List, "<<numbersCount<<"items"<<repetitions<<"repetitions";

    QList<int> list;

    for(int i=0;i<numbersCount;i++)
    {

        list+=i;
     }

    TNS__NumberList nl;
    nl.setNumber(list);
    TNS__NumberList retListOfNumbers;
    QDateTime time=QDateTime::currentDateTime();
    for(int i=0;i<repetitions;i++)
    {
        retListOfNumbers=client->listOfNumbers(nl);

    }
    QDateTime timeEnd=QDateTime::currentDateTime();
    qint64 delta=time.msecsTo(timeEnd);
    qDebug()<<"Took "<<delta<<"msec";

    if(DEBUG_LEVEL>1)
    {
        QListIterator<int> it(retListOfNumbers.number());
        QString returnNumbers="";
        while(it.hasNext())
        {
            int s=it.next();
            returnNumbers+=" "+QString::number(s);
        }
        qDebug()<<"Return :"<<returnNumbers;
    }
    
}

void testStringList(int stringCount,QString s,int repetitions, IntegrationTest * client)
{
    qDebug()<<"Test: Large String List, "<<stringCount<<"items,"<<repetitions<<"repetitions, string:"<<s;

    QList<QString> list;

    for(int i;i<stringCount;i++)
    {
        list<<s;
    }

    TNS__StringList nl;
    nl.setString(list);

    TNS__StringList retListOfStrings;
    QDateTime time=QDateTime::currentDateTime();
    for(int i=0;i<repetitions;i++)
    {
        retListOfStrings=client->listOfStrings(nl);

    }

    QDateTime timeEnd=QDateTime::currentDateTime();
    qint64 delta=time.msecsTo(timeEnd);
    qDebug()<<"Took "<<delta<<"msec";
   
}

void testNumber(int repetitions, IntegrationTest * client)
{
    qDebug()<<"Test: Number transfer, "<<repetitions<<"repetitions";


    QDateTime time=QDateTime::currentDateTime();

    for(int i=0;i<repetitions;i++)
    {
        int r=client->numberTransfer(i);

    }

    QDateTime timeEnd=QDateTime::currentDateTime();
    qint64 delta=time.msecsTo(timeEnd);
    qDebug()<<"Took "<<delta<<"msec";
   
}

void testString(int stringLength, int repetitions, IntegrationTest * client)
{
    qDebug()<<"Test: String transfer, "<<stringLength<<"is the string length,"<<repetitions<<"repetitions";

    QString s="X";
    s=s.leftJustified(stringLength,'.');
    QDateTime time=QDateTime::currentDateTime();

    for(int i=0;i<repetitions;i++)
    {
        QString r=client->stringTransfer(s);

    }

    QDateTime timeEnd=QDateTime::currentDateTime();
    qint64 delta=time.msecsTo(timeEnd);
    qDebug()<<"Took "<<delta<<"msec";
    
}

void testContainer(int repetitions, IntegrationTest * client)
{
    qDebug()<<"Test: Container transfer, "<<repetitions<<"repetitions";


    TNS__NumberList nl;
    QList<int> nList;
    nList<<1<<2<<3;
    nl.setNumber(nList);
    TNS__ContainerType ct;
    ct.setDateTime(QDateTime::currentDateTime());
    ct.setString("Hello Server");
    ct.setNumberList(nl);

    QDateTime time=QDateTime::currentDateTime();

    for(int i=0;i<repetitions;i++)
    {
        TNS__ContainerType s=client->containerTransfer(ct);

    }

    QDateTime timeEnd=QDateTime::currentDateTime();
    qint64 delta=time.msecsTo(timeEnd);
    qDebug()<<"Took "<<delta<<"msec";
    
}


IntegrationTest * createClient(QString url)
{

    IntegrationTest * client=new IntegrationTest;

    client->setEndPoint(url);
    return client;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug()<<"Using service url \""<<SERVICEURL<<"\"";


    IntegrationTest * client=createClient(SERVICEURL);

    qDebug()<<"Firstcall ";//so we do not take the time for the first connection
    int b = client->numberTransfer(10);

    testNumber(TEST_REPETITIONS,client);

    testString(100,TEST_REPETITIONS,client);

    testNumbersList(1000,TEST_REPETITIONS,client);// 300 items is to much for the Server, 5000 seg-vaults the client    

    testStringList(10,"Hello WorldHello WorldHello WorldHello WorldHello World",100,client);

    testContainer(TEST_REPETITIONS,client);
    qDebug()<<"Finished";
    return a.exec();
}
