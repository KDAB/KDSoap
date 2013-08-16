/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "wsdl_resourceinteraction.h"

#include "httpserver_p.h"
#include <QtTest/QtTest>
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

    virtual TNS__WSResourceValueEnvelope getRuntimeValue( int ) {
        return TNS__WSResourceValueEnvelope(); }
    virtual TNS__ArrayOfWSResourceValueEnvelope getRuntimeValues( const XSD__ArrayOfint& ) {
        return TNS__ArrayOfWSResourceValueEnvelope(); }
    virtual TNS__ArrayOfWSResourceValueEnvelope getInitialValues( const XSD__ArrayOfint& ) {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual bool setResourceValue( const TNS__WSResourceValueEnvelope& parameter4 ) {
        qDebug() << parameter4.typeString();
        //qDebug() << dynamic_cast<WPNS1__WSEnumValue *>(&parameter4.value());
        return (parameter4.typeString() == "type");
    }
    virtual bool setResourceValues( const TNS__ArrayOfWSResourceValueEnvelope& ) {
        return false;
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope enableRuntimeValueNotifications( const XSD__ArrayOfint& ) {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual bool disableRuntimeValueNotifactions( const XSD__ArrayOfint& ) {
        return false;
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope enableInitialValueNotifications( const XSD__ArrayOfint& ) {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual bool disableInitialValueNotifactions( const XSD__ArrayOfint& ) {
        return false;
    }
    virtual TNS__ArrayOfWSResourceValueEnvelope waitForResourceValueChanges( int ) {
        return TNS__ArrayOfWSResourceValueEnvelope();
    }
    virtual TNS__ArrayOfWSSceneResourceIdAndLocationURLs getSceneGroupResourceIdAndPositions( int ) {
        return TNS__ArrayOfWSSceneResourceIdAndLocationURLs();
    }
    virtual TNS__WSSceneResourceIdAndLocationURLs getScenePositionsForSceneValueResource( int ) {
        return TNS__WSSceneResourceIdAndLocationURLs();
    }
    virtual WPNS1__ArrayOfWSEnumDefinition getEnumeratorDefinitions() {
        return WPNS1__ArrayOfWSEnumDefinition();
    }
    virtual QString getResourceType( int ) {
        return QString();
    }
    virtual TNS__ArrayOfWSDatalineResource getExtraDatalineInputs() {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__ArrayOfWSDatalineResource getExtraDatalineOutputs() {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__ArrayOfWSDatalineResource getAllDatalineInputs() {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__ArrayOfWSDatalineResource getAllDatalineOutputs() {
        return TNS__ArrayOfWSDatalineResource();
    }
    virtual TNS__WSResourceValueEnvelope getInitialValue( int ) {
        return TNS__WSResourceValueEnvelope();
    }
};

class IHCServer : public KDSoapServer
{
    Q_OBJECT
public:
    IHCServer() : KDSoapServer(), m_lastServerObject(0) {
    }
    virtual QObject* createServerObject() { m_lastServerObject = new IHCServerObject; return m_lastServerObject; }
    IHCServerObject* lastServerObject() { return m_lastServerObject; }
private:
    IHCServerObject* m_lastServerObject;
};

class IHCResourceInteractionTest : public QObject
{
    Q_OBJECT

private:

private Q_SLOTS:

    void testInheritance()
    {
        TestServerThread<IHCServer> serverThread;
        IHCServer* server = serverThread.startThread();

        ResourceInteractionServiceService service;
        service.setEndPoint(server->endPoint());

        TNS__WSResourceValueEnvelope env;
        env.setResourceID(54);
        env.setTypeString(QLatin1String("type"));
        WPNS1__WSEnumValue myEnum;
        myEnum.setDefinitionTypeID(2);
        myEnum.setEnumName("enum1");
        myEnum.setEnumValueID(42);
        env.setValue(myEnum);
        qDebug() << dynamic_cast<WPNS1__WSEnumValue *>(&myEnum);
        service.setResourceValue(env);

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
};

QTEST_MAIN(IHCResourceInteractionTest)

#include "test_ihc.moc"
