#include <wsdl.h>

#include <fileprovider.h>
#include <messagehandler.h>
#include <parsercontext.h>

#include <converter.h>
#include <creator.h>
#include <settings.h>

#include <compiler.h>
//#include "wsdl_import_definition.h"
#include "httpserver_p.h"

#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>


using namespace KDSoapUnitTestHelpers;

class ListRestrictionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGenerateMethodSetValueOnClassD3ArrayType()
    {
        QString generateStr, strToCompare;

        FileProvider provider;

        QString fileName = "test_list_restriction.wsdl";

        QFile file(fileName);
        //file.open();


            //qDebug() << "parsing" << fileName;
            QXmlInputSource source(&file);
            QXmlSimpleReader reader;
            reader.setFeature(QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true);

            QString errorMsg;
            int errorLine, errorCol;
            QDomDocument doc;
           doc.setContent(&source, &reader, &errorMsg, &errorLine, &errorCol);



        QDomElement element = doc.documentElement();



        NSManager namespaceManager;

        // we don't parse xml.xsd, so hardcode the xml prefix (for xml:lang)
        namespaceManager.setPrefix(QLatin1String("xml"), NSManager::xmlNamespace());

        MessageHandler messageHandler;
        ParserContext context;
        context.setNamespaceManager(&namespaceManager);
        context.setMessageHandler(&messageHandler);
        //context.setDocumentBaseUrl(QUrl(Settings::self()->wsdlBaseUrl()));

        KWSDL::Definitions definitions;
        //definitions.setWantedService(Settings::self()->wantedService());
        if (definitions.loadXML(&context, element)) {

            definitions.fixUpDefinitions(/*&context, element*/);

            KODE::Code::setDefaultIndentation(4);

            KWSDL::WSDL wsdl;
            wsdl.setDefinitions(definitions);
            wsdl.setNamespaceManager(namespaceManager);

            KWSDL::Converter converter;
            converter.setWSDL(wsdl);

            converter.convert();
            foreach (auto elem, converter.classes()) {
                if(elem.name() == QString("MODEL__D3ArrayType")) {
                    foreach(auto e_f,elem.functions()) {
                        if(e_f.name() == QString("setValue")) {
                            generateStr = e_f.body();
                        }
                    }
                }
            }
        }
        strToCompare = "bool rangeOk = true;\n\nrangeOk = rangeOk && (value.entries().length() == 3);\n\nif(!rangeOk)\n\tqDebug( \"Invalid range in MODEL__D3ArrayType::setValue()\" );\n\nmValue = value;\n";
        QCOMPARE(generateStr, strToCompare);
    }
};

QTEST_MAIN(ListRestrictionTest)

#include "test_list_restriction.moc"
