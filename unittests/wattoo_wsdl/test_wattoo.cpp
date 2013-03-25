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
** GNU Lesser General Public License version 2 and version 3 as published by the
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

class WattooTest : public QObject
{
    Q_OBJECT

private:

private Q_SLOTS:

    void testInheritance()
    {
        ResourceInteractionServiceService service;
        if (false) { // check compilation, not runtime
            //service.setEndPoint(server.endPoint());
            TNS__WSResourceValueEnvelope env;
            env.setResourceID(54);
            env.setTypeString(QLatin1String("type"));
            WPNS1__WSEnumValue myEnum;
            myEnum.setDefinitionTypeID(2);
            myEnum.setEnumName("enum1");
            myEnum.setEnumValueID(42);
            env.setValue(myEnum);
            service.setResourceValue(env);
        }
    }
};

QTEST_MAIN(WattooTest)

#include "test_wattoo.moc"
