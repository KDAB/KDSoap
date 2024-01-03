/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "wsdl_kdtest.h"

#include "httpserver_p.h"
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapNamespaceManager.h>
#include <KDSoapServer.h>
#include <QDebug>
#include <QEventLoop>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

// Server side to perform job operation request
class GuestServerObject : public GuestServerBase /* generated from wsdl */
{
    Q_OBJECT
public:
    virtual TNS__AddPersonResponse addPerson(const TNS__AddPersonRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__AddPersonResponse();
    }
    virtual TNS__AuthenticateResponse authenticate(const TNS__AuthenticateRequest &parameters) override
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
    virtual TNS__SearchPersonResponse searchPerson(const TNS__SearchPersonRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__SearchPersonResponse();
    }
    virtual TNS__EditPersonResponse editPerson(const TNS__EditPersonRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__EditPersonResponse();
    }
    virtual TNS__CityListResponse cityList(const TNS__CityListRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__CityListResponse();
    }
    virtual TNS__PlaceListResponse placeList(const TNS__PlaceListRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__PlaceListResponse();
    }
    virtual TNS__DocumentListResponse documentList(const TNS__DocumentListRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__DocumentListResponse();
    }
    virtual TNS__GuestTypeListResponse guestTypeList(const TNS__GuestTypeListRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__GuestTypeListResponse();
    }
    virtual TNS__CountrytListResponse countrytList(const TNS__CountrytListRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__CountrytListResponse();
    }
    virtual TNS__BuildingListResponse buildingList(const TNS__BuildingListRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__BuildingListResponse();
    }
    virtual TNS__EntranceListResponse entranceList(const TNS__EntranceListRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__EntranceListResponse();
    }
    virtual TNS__ServiceResponse service(const TNS__ServiceRequest &parameters) override
    {
        Q_UNUSED(parameters);
        return TNS__ServiceResponse();
    }
};

class GuestServer : public KDSoapServer
{
    Q_OBJECT
public:
    GuestServer()
        : KDSoapServer()
        , m_lastServerObject(0)
    {
        setPath(QLatin1String("/xml"));
    }
    virtual QObject *createServerObject() override
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
        connect(job, &AuthenticateJob::finished, this, &PrefixTest::slotAuthenticateJobFinished);
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
