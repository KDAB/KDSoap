/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "wsdl_resourceinteraction.h"

#include "httpserver_p.h"
#include <QTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapServer.h>
#include <KDSoapNamespaceManager.h>

using namespace KDSoapUnitTestHelpers;

class IHCServerObject : public ResourceInteractionServiceServiceServerBase
{
public:

    virtual TNS__WSResourceValueEnvelope getRuntimeValue(int) override
    {
        return TNS__WSResourceValueEnvelope();
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope getRuntimeValues(const XSD__ArrayOfint &) override
    {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope getInitialValues(const XSD__ArrayOfint &) override
    {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual bool setResourceValue(const TNS__WSResourceValueEnvelope &parameter4) override
    {
        Q_ASSERT(parameter4.typeString() == "enum");
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-undefined-compare"
#endif
        Q_ASSERT(&parameter4.value() != 0);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

        const KDSoapValue value = parameter4.value_as_kdsoap_value();

        // Custom deserialization: given the typeString() we know we can create a WPNS1__WSEnumValue
        WPNS1__WSEnumValue enumValue;
        enumValue.deserialize(value);
        Q_ASSERT(enumValue.enumValueID() == 42);

        // Alternatively: dynamic introspection

        const KDSoapValueList children = value.childValues();
        const int enumValueID = children.child("enumValueID").value().toInt();
        return (parameter4.typeString() == "enum" && enumValueID == 42 && enumValue.enumValueID() == 42);

        // This cannot be done, the generated code can't know which subclass to create, when using "literal"
        // ("encoded" would give us the information, but isn't used here)
        /*
        const WPNS1__WSEnumValue *inputValue = dynamic_cast<const WPNS1__WSEnumValue *>(&parameter4.value());
        qDebug() << inputValue;
        Q_ASSERT(inputValue);
        Q_ASSERT(inputValue->enumValueID() == 42);
        return (parameter4.typeString() == "enum" && inputValue->enumValueID() == 42);
        */
    }
    virtual bool setResourceValues(const TNS__ArrayOfWSResourceValueEnvelope &) override
    {
        return false;
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope enableRuntimeValueNotifications(const XSD__ArrayOfint &) override
    {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual bool disableRuntimeValueNotifactions(const XSD__ArrayOfint &) override
    {
        return false;
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope enableInitialValueNotifications(const XSD__ArrayOfint &) override
    {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual bool disableInitialValueNotifactions(const XSD__ArrayOfint &) override
    {
        return false;
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope waitForResourceValueChanges(int) override
    {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual TNS__ArrayOfWSSceneResourceIdAndLocationURLs getSceneGroupResourceIdAndPositions(int) override
    {
        return TNS__ArrayOfWSSceneResourceIdAndLocationURLs();
    }
    virtual TNS__WSSceneResourceIdAndLocationURLs getScenePositionsForSceneValueResource(int) override
    {
        return TNS__WSSceneResourceIdAndLocationURLs();
    }
    virtual WPNS1__ArrayOfWSEnumDefinition getEnumeratorDefinitions() override
    {
        return WPNS1__ArrayOfWSEnumDefinition();
    }
    virtual QString getResourceType(int) override
    {
        return QString();
    }
    virtual TNS__ArrayOfWSDatalineResource getExtraDatalineInputs() override
    {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__ArrayOfWSDatalineResource getExtraDatalineOutputs() override
    {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__ArrayOfWSDatalineResource getAllDatalineInputs() override
    {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__ArrayOfWSDatalineResource getAllDatalineOutputs() override
    {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__WSResourceValueEnvelope getInitialValue(int) override
    {
        return TNS__WSResourceValueEnvelope();
    }
};

class IHCServer : public KDSoapServer
{
    Q_OBJECT
public:
    IHCServer() : KDSoapServer(), m_lastServerObject(0)
    {
    }
    virtual QObject *createServerObject() override
    {
        m_lastServerObject = new IHCServerObject;
        return m_lastServerObject;
    }
    IHCServerObject *lastServerObject()
    {
        return m_lastServerObject;
    }
private:
    IHCServerObject *m_lastServerObject;
};

class IHCResourceInteractionTest : public QObject
{
    Q_OBJECT

private:

private Q_SLOTS:

    void testInheritanceClientSide()
    {
        HttpServerThread server(fakeResponse(), HttpServerThread::Public);

        ResourceInteractionServiceService service;
        service.setEndPoint(server.endPoint());

        TNS__WSResourceValueEnvelope env = resourceValueEnvelope();

        // check that we can extract the derived class again, i.e. it was correctly stored
        const WPNS1__WSEnumValue *storedValue = dynamic_cast<const WPNS1__WSEnumValue *>(&env.value());
        QVERIFY(storedValue);
        QCOMPARE(storedValue->enumValueID(), 42);

        service.setResourceValue(env);

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedSetResourceValueRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedSetResourceValueRequest().constData()));
            QCOMPARE(server.header("SoapAction").constData(), "\"setResourceValue\"");
        }
    }

    void testInheritanceServerSide()
    {
        TestServerThread<IHCServer> serverThread;
        IHCServer *server = serverThread.startThread();

        ResourceInteractionServiceService service;
        service.setEndPoint(server->endPoint());

        TNS__WSResourceValueEnvelope env = resourceValueEnvelope();

        // check that we can extract the derived class again, i.e. it was correctly stored
        const WPNS1__WSEnumValue *storedValue = dynamic_cast<const WPNS1__WSEnumValue *>(&env.value());
        QVERIFY(storedValue);
        QCOMPARE(storedValue->enumValueID(), 42);

        bool ret = service.setResourceValue(env);

        QVERIFY(ret);
        //qDebug() << server->lastServerObject()-> ...;
    }

    void testBuiltinInitialization()
    {
        QCOMPARE(SetResourceValueJob(0, 0).return_(), false);
        QCOMPARE(SetResourceValuesJob(0, 0).return_(), false);
        QCOMPARE(DisableRuntimeValueNotifactionsJob(0, 0).return_(), false);
        QCOMPARE(DisableInitialValueNotifactionsJob(0, 0).return_(), false);
        QCOMPARE(GetRuntimeValueJob(0, 0).parameter1(), 0);
        QCOMPARE(WaitForResourceValueChangesJob(0, 0).parameter10(), 0);
        QCOMPARE(GetSceneGroupResourceIdAndPositionsJob(0, 0).parameter11(), 0);
        QCOMPARE(GetScenePositionsForSceneValueResourceJob(0, 0).parameter12(), 0);
        QCOMPARE(GetResourceTypeJob(0, 0).parameter13(), 0);
        QCOMPARE(GetInitialValueJob(0, 0).parameter14(), 0);
    }

private:

    static TNS__WSResourceValueEnvelope resourceValueEnvelope()
    {
        TNS__WSResourceValueEnvelope env;
        env.setResourceID(54);
        env.setTypeString(QLatin1String("enum"));
        WPNS1__WSEnumValue myEnum; // derives from WSResourceValue
        myEnum.setDefinitionTypeID(2);
        myEnum.setEnumName("enum1");
        myEnum.setEnumValueID(42);
        env.setValue(myEnum); // takes a WSResourceValue

        return env;
    }

    static QByteArray expectedSetResourceValueRequest()
    {
        return QByteArray(xmlEnvBegin11()) + ">"
               "<soap:Body>"
               "<n1:setResourceValue1 xmlns:n1=\"utcs\">"
               "<n1:resourceID>54</n1:resourceID>"
               "<n1:isValueRuntime>false</n1:isValueRuntime>"
               "<n1:typeString>enum</n1:typeString>"
               "<n1:value>"
               "<n2:definitionTypeID xmlns:n2=\"utcs.values\">2</n2:definitionTypeID>"
               "<n3:enumValueID xmlns:n3=\"utcs.values\">42</n3:enumValueID>"
               "<n4:enumName xmlns:n4=\"utcs.values\">enum1</n4:enumName>"
               "</n1:value>"
               "</n1:setResourceValue1>"
               "</soap:Body>" + xmlEnvEnd()
               + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }

    static QByteArray fakeResponse()
    {
        return QByteArray(xmlEnvBegin11()) + ">"
               "<soap:Body>"
               " </soap:Body>" + xmlEnvEnd();
    }
};

QTEST_MAIN(IHCResourceInteractionTest)

#include "test_ihc.moc"
