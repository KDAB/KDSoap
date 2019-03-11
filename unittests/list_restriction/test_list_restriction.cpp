#include <wsdl.h>

#include <messagehandler.h>
#include <parsercontext.h>

#include <converter.h>
#include <creator.h>
#include <settings.h>

#include <compiler.h>
#include "httpserver_p.h"

#include <QtTest>


using namespace KDSoapUnitTestHelpers;

class ListRestrictionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void getFunctionCodeBody(const QByteArray & srcXml, QString & generatedStr) 
    {
        generatedStr = "";
        QStringList generatedFunctionBody;
        
        QXmlInputSource source;
        source.setData(srcXml);
        QXmlSimpleReader reader;
        reader.setFeature(QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true);

        QString errorMsg;
        int errorLine, errorCol;
        QDomDocument doc;
        QVERIFY(doc.setContent(&source, &reader, &errorMsg, &errorLine, &errorCol));

        QDomElement element = doc.documentElement();
        NSManager namespaceManager;
        namespaceManager.setPrefix(QLatin1String("xml"), NSManager::xmlNamespace());

        MessageHandler messageHandler;
        ParserContext context;
        context.setNamespaceManager(&namespaceManager);
        context.setMessageHandler(&messageHandler);

        KWSDL::Definitions definitions;
        QVERIFY(definitions.loadXML(&context, element));
        definitions.fixUpDefinitions();        
        KODE::Code::setDefaultIndentation(4);
        
        KWSDL::WSDL wsdl;
        wsdl.setDefinitions(definitions);
        wsdl.setNamespaceManager(namespaceManager);
        
        KWSDL::Converter converter;
        converter.setWSDL(wsdl);
        converter.convert();
        
        foreach (auto elem, converter.classes()) {
            if (elem.name() == QString("TEST__D3ArrayType")) {
                foreach (auto e_f, elem.functions()) {
                    if (e_f.name() == QString("setValue")) {
                        generatedFunctionBody = e_f.body().split("\n");
                        generatedStr = generatedFunctionBody.at(2);
                    }
                }
            }
        }
    }

    void testGenerateCodeWithRestriction()
    {
        QString generatedCodeString;
        QString expectedSrcListRestriction = "rangeOk = rangeOk && (value.entries().length() == 3);";
        QString expectedSrcStringRestriction = "rangeOk = rangeOk && (value.value().length() == 3);";
        QString expectedSrcXSDListRestriction = "rangeOk = rangeOk && (value.length() == 3);";

        const QByteArray srcXmlListRestriction = QByteArray("<definitions name=\"HelloService\""
                                             "   targetNamespace=\"http://www.examples.com/wsdl/HelloService.wsdl\""
                                             "   xmlns=\"http://schemas.xmlsoap.org/wsdl/\""
                                             "   xmlns:soap=\"http://schemas.xmlsoap.org/wsdl/soap/\""
                                             "   xmlns:tns=\"http://www.examples.com/wsdl/HelloService.wsdl\""
                                             "   xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                                             "   xmlns:test=\"urn:test\">"
                                             "   <types>"
                                             "    <xsd:schema targetNamespace=\"urn:test\""
                                             "            xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" >"
                                             "            <xsd:import namespace=\"http://www.w3.org/2001/XMLSchema\"/>  "
                                             "        <xsd:simpleType name=\"dArrayType\">"
                                             "           <xsd:list itemType=\"xsd:double\"/>"
                                             "        </xsd:simpleType>"
                                             "        <xsd:simpleType name=\"d3ArrayType\">"
                                             "            <xsd:restriction base=\"test:dArrayType\">"
                                             "                <xsd:length value=\"3\"/>"
                                             "            </xsd:restriction>"
                                             "        </xsd:simpleType>"
                                             "    </xsd:schema>"
                                             "    </types>"
                                             "   <message name=\"SayHelloRequest\">"
                                             "      <part name=\"firstName\" type=\"test:d3ArrayType\"/>"
                                             "   </message>"
                                             "   <message name=\"SayHelloResponse\">"
                                             "      <part name=\"greeting\" type=\"xsd:string\"/>"
                                             "   </message>"
                                             "   <portType name=\"Hello_PortType\">"
                                             "      <operation name=\"sayHello\">"
                                             "         <input message=\"tns:SayHelloRequest\"/>"
                                             "         <output message=\"tns:SayHelloResponse\"/>"
                                             "      </operation>"
                                             "   </portType>"
                                             "   <binding name=\"Hello_Binding\" type=\"tns:Hello_PortType\">"
                                             "   <soap:binding style=\"rpc\""
                                             "      transport=\"http://schemas.xmlsoap.org/soap/http\"/>"
                                             "   <operation name=\"sayHello\">"
                                             "      <soap:operation soapAction=\"sayHello\"/>"
                                             "      <input>"
                                             "         <soap:body"
                                             "            encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\""
                                             "            namespace=\"urn:examples:helloservice\""
                                             "            use=\"encoded\"/>"
                                             "      </input>"
                                             "      <output>"
                                             "         <soap:body"
                                             "            encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\""
                                             "            namespace=\"urn:examples:helloservice\""
                                             "            use=\"encoded\"/>"
                                             "      </output>"
                                             "   </operation>"
                                             "   </binding> "
                                             "   <service name=\"Hello_Service\">"
                                             "      <documentation>WSDL File for HelloService</documentation>"
                                             "      <port binding=\"tns:Hello_Binding\" name=\"Hello_Port\">"
                                             "         <soap:address"
                                             "            location=\"http://www.examples.com/SayHello/\"/>"
                                             "      </port>"
                                             "   </service>"
                                             "</definitions>");
        
        const QByteArray srcXmlStringRestriction = QByteArray("<definitions name=\"HelloService\""
                                             "   targetNamespace=\"http://www.examples.com/wsdl/HelloService.wsdl\""
                                             "   xmlns=\"http://schemas.xmlsoap.org/wsdl/\""
                                             "   xmlns:soap=\"http://schemas.xmlsoap.org/wsdl/soap/\""
                                             "   xmlns:tns=\"http://www.examples.com/wsdl/HelloService.wsdl\""
                                             "   xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                                             "   xmlns:test=\"urn:test\">"
                                             "   <types>"
                                             "    <xsd:schema targetNamespace=\"urn:test\""
                                             "            xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" >"
                                             "            <xsd:import namespace=\"http://www.w3.org/2001/XMLSchema\"/>  "
                                             "        <xsd:simpleType name=\"dArrayType\">"
                                             "           <xsd:element type=\"xsd:string\" />"
                                             "        </xsd:simpleType>"
                                             "        <xsd:simpleType name=\"d3ArrayType\">"
                                             "            <xsd:restriction base=\"test:dArrayType\">"
                                             "                <xsd:length value=\"3\"/>"
                                             "            </xsd:restriction>"
                                             "        </xsd:simpleType>"
                                             "    </xsd:schema>"
                                             "    </types>"
                                             "   <message name=\"SayHelloRequest\">"
                                             "      <part name=\"firstName\" type=\"test:d3ArrayType\"/>"
                                             "   </message>"
                                             "   <message name=\"SayHelloResponse\">"
                                             "      <part name=\"greeting\" type=\"xsd:string\"/>"
                                             "   </message>"
                                             "   <portType name=\"Hello_PortType\">"
                                             "      <operation name=\"sayHello\">"
                                             "         <input message=\"tns:SayHelloRequest\"/>"
                                             "         <output message=\"tns:SayHelloResponse\"/>"
                                             "      </operation>"
                                             "   </portType>"
                                             "   <binding name=\"Hello_Binding\" type=\"tns:Hello_PortType\">"
                                             "   <soap:binding style=\"rpc\""
                                             "      transport=\"http://schemas.xmlsoap.org/soap/http\"/>"
                                             "   <operation name=\"sayHello\">"
                                             "      <soap:operation soapAction=\"sayHello\"/>"
                                             "      <input>"
                                             "         <soap:body"
                                             "            encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\""
                                             "            namespace=\"urn:examples:helloservice\""
                                             "            use=\"encoded\"/>"
                                             "      </input>"
                                             "      <output>"
                                             "         <soap:body"
                                             "            encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\""
                                             "            namespace=\"urn:examples:helloservice\""
                                             "            use=\"encoded\"/>"
                                             "      </output>"
                                             "   </operation>"
                                             "   </binding> "
                                             "   <service name=\"Hello_Service\">"
                                             "      <documentation>WSDL File for HelloService</documentation>"
                                             "      <port binding=\"tns:Hello_Binding\" name=\"Hello_Port\">"
                                             "         <soap:address"
                                             "            location=\"http://www.examples.com/SayHello/\"/>"
                                             "      </port>"
                                             "   </service>"
                                             "</definitions>");

        const QByteArray srcXmlXSDListRestriction = QByteArray("<definitions name=\"HelloService\""
                                             "   targetNamespace=\"http://www.examples.com/wsdl/HelloService.wsdl\""
                                             "   xmlns=\"http://schemas.xmlsoap.org/wsdl/\""
                                             "   xmlns:soap=\"http://schemas.xmlsoap.org/wsdl/soap/\""
                                             "   xmlns:tns=\"http://www.examples.com/wsdl/HelloService.wsdl\""
                                             "   xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                                             "   xmlns:test=\"urn:test\">"
                                             "   <types>"
                                             "    <xsd:schema targetNamespace=\"urn:test\""
                                             "            xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" >"
                                             "            <xsd:import namespace=\"http://www.w3.org/2001/XMLSchema\"/>  "
                                             "        <xsd:simpleType name=\"d3ArrayType\">"
                                             "            <xsd:restriction base=\"xsd:string\">"
                                             "                <xsd:length value=\"3\"/>"
                                             "            </xsd:restriction>"
                                             "        </xsd:simpleType>"
                                             "    </xsd:schema>"
                                             "    </types>"
                                             "   <message name=\"SayHelloRequest\">"
                                             "      <part name=\"firstName\" type=\"test:d3ArrayType\"/>"
                                             "   </message>"
                                             "   <message name=\"SayHelloResponse\">"
                                             "      <part name=\"greeting\" type=\"xsd:string\"/>"
                                             "   </message>"
                                             "   <portType name=\"Hello_PortType\">"
                                             "      <operation name=\"sayHello\">"
                                             "         <input message=\"tns:SayHelloRequest\"/>"
                                             "         <output message=\"tns:SayHelloResponse\"/>"
                                             "      </operation>"
                                             "   </portType>"
                                             "   <binding name=\"Hello_Binding\" type=\"tns:Hello_PortType\">"
                                             "   <soap:binding style=\"rpc\""
                                             "      transport=\"http://schemas.xmlsoap.org/soap/http\"/>"
                                             "   <operation name=\"sayHello\">"
                                             "      <soap:operation soapAction=\"sayHello\"/>"
                                             "      <input>"
                                             "         <soap:body"
                                             "            encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\""
                                             "            namespace=\"urn:examples:helloservice\""
                                             "            use=\"encoded\"/>"
                                             "      </input>"
                                             "      <output>"
                                             "         <soap:body"
                                             "            encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\""
                                             "            namespace=\"urn:examples:helloservice\""
                                             "            use=\"encoded\"/>"
                                             "      </output>"
                                             "   </operation>"
                                             "   </binding> "
                                             "   <service name=\"Hello_Service\">"
                                             "      <documentation>WSDL File for HelloService</documentation>"
                                             "      <port binding=\"tns:Hello_Binding\" name=\"Hello_Port\">"
                                             "         <soap:address"
                                             "            location=\"http://www.examples.com/SayHello/\"/>"
                                             "      </port>"
                                             "   </service>"
                                             "</definitions>");

        getFunctionCodeBody(srcXmlListRestriction, generatedCodeString);
        QCOMPARE(expectedSrcListRestriction, generatedCodeString);

        getFunctionCodeBody(srcXmlStringRestriction, generatedCodeString);
        QCOMPARE(expectedSrcStringRestriction, generatedCodeString);

        getFunctionCodeBody(srcXmlXSDListRestriction, generatedCodeString);
        QCOMPARE(expectedSrcXSDListRestriction, generatedCodeString);
    }
};

QTEST_MAIN(ListRestrictionTest)

#include "test_list_restriction.moc"
