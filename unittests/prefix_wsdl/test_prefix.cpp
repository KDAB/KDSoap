/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
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

#include "wsdl_kdtest.h"

#include "httpserver_p.h"
#include <QTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapServer.h>
#include <KDSoapNamespaceManager.h>

using namespace KDSoapUnitTestHelpers;

// Server side to perform job operation request
class GuestServerObject : public GuestServerBase /* generated from wsdl */
{
    Q_OBJECT
public:
    virtual TNS__AddPersonResponse addPerson(const TNS__AddPersonRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__AddPersonResponse();
    }
    virtual TNS__AuthenticateResponse authenticate(const TNS__AuthenticateRequest &parameters)
    {
        if (parameters.serijskiBrojSertifikata() == "cert") {
            TNS__AuthenticateResponse response;
            response.setResult("result");
            TNS__User user;
            user.setFullName("userFullName");
            response.setUser(user);
            return response;
        }
        return TNS__AuthenticateResponse();
    }
    virtual TNS__SearchPersonResponse searchPerson(const TNS__SearchPersonRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__SearchPersonResponse();
    }
    virtual TNS__EditPersonResponse editPerson(const TNS__EditPersonRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__EditPersonResponse();
    }
    virtual TNS__CityListResponse cityList(const TNS__CityListRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__CityListResponse();
    }
    virtual TNS__PlaceListResponse placeList(const TNS__PlaceListRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__PlaceListResponse();
    }
    virtual TNS__DocumentListResponse documentList(const TNS__DocumentListRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__DocumentListResponse();
    }
    virtual TNS__GuestTypeListResponse guestTypeList(const TNS__GuestTypeListRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__GuestTypeListResponse();
    }
    virtual TNS__CountrytListResponse countrytList(const TNS__CountrytListRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__CountrytListResponse();
    }
    virtual TNS__BuildingListResponse buildingList(const TNS__BuildingListRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__BuildingListResponse();
    }
    virtual TNS__EntranceListResponse entranceList(const TNS__EntranceListRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__EntranceListResponse();
    }
    virtual TNS__ServiceResponse service(const TNS__ServiceRequest &parameters)
    {
        Q_UNUSED(parameters);
        return TNS__ServiceResponse();
    }
};

class GuestServer : public KDSoapServer
{
    Q_OBJECT
public:
    GuestServer() : KDSoapServer(), m_lastServerObject(0)
    {
        setPath(QLatin1String("/xml"));
    }
    virtual QObject *createServerObject()
    {
        m_lastServerObject = new GuestServerObject;
        return m_lastServerObject;
    }

    GuestServerObject *lastServerObject()
    {
        return m_lastServerObject;
    }

private:
    GuestServerObject *m_lastServerObject; // only for unittest purposes
};

class PrefixTest : public QObject
{
    Q_OBJECT

private slots:

    void testTransformJob()
    {
        TestServerThread<GuestServer> serverThread;
        GuestServer *server = serverThread.startThread();

        Guest service;
        service.setEndPoint(server->endPoint());

        AuthenticateJob *job = new AuthenticateJob(&service, this);
        TNS__AuthenticateRequest req;
        req.setSerijskiBrojSertifikata("cert");
        job->setParameters(req);
        connect(job, SIGNAL(finished(KDSoapJob*)), this, SLOT(slotAuthenticateJobFinished(KDSoapJob*)));
        job->start();
        m_eventLoop.exec();

        TNS__AuthenticateResponse response = job->resultParameters();
        QCOMPARE(response.user().fullName(), QString("userFullName"));
        QCOMPARE(response.result(), QString("result"));
    }

protected slots: // the really private ones

    void slotAuthenticateJobFinished(KDSoapJob *)
    {
        m_eventLoop.quit();
    }
private:
    QEventLoop m_eventLoop;
};

QTEST_MAIN(PrefixTest)

#include "test_prefix.moc"
