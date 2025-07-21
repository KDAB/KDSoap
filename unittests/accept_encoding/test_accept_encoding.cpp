/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2025 Jonathan Brady <jtjbrady@users.noreply.github.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapServer.h"
#include "KDSoapServerObjectInterface.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QtTest>

class FileServerObject final : public QObject, public KDSoapServerObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)
public:
    FileServerObject() = default;
    ~FileServerObject() = default;

    HttpResponseHeaderItems additionalHttpResponseHeaderItems() const override
    {
        if (!m_contentEncoding.isEmpty())
            return {{"Content-Encoding", m_contentEncoding}};
        return {};
    }

    QIODevice *processFileRequest(const QString &path, QByteArray &contentType) override
    {
        const auto [device, encodingOrError] = fileForEncoding("searchpath:" + path);

        if (!device) {
            if (!encodingOrError.isEmpty()) {
                writeHTTP(encodingOrError);
                // We set a fault so KDSoapServerSocket doesn't add a not found error
                // The first parameters are ignored but if the first one is null setFault
                // will assert.
                setFault(QStringLiteral("HTTP"), QString());
            }
            return nullptr;
        }

        m_contentEncoding = encodingOrError;
        contentType = "text/plain";
        return device;
    }

private:
    QByteArray m_contentEncoding;
};

class FileServer final : public KDSoapServer
{
    Q_OBJECT
public:
    FileServer() = default;
    ~FileServer() = default;

    QObject *createServerObject() override
    {
        return new FileServerObject();
    }
};

class TestAcceptEncoding : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void encodingScenarios_data();
    void encodingScenarios();

private:
    FileServer *server = nullptr;
    QNetworkAccessManager *manager = nullptr;
};

void TestAcceptEncoding::initTestCase()
{
    QDir::setSearchPaths("searchpath", {":/"});
    server = new FileServer();
    QVERIFY(server->listen(QHostAddress::LocalHost));

    manager = new QNetworkAccessManager(this);
}

void TestAcceptEncoding::cleanupTestCase()
{
    delete server;
    delete manager;
}

void TestAcceptEncoding::encodingScenarios_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QByteArray>("acceptEncoding");
    QTest::addColumn<int>("expectedStatus");
    QTest::addColumn<QByteArray>("expectedEncoding");
    /*
        // File from disk (uncompressed)
        QTest::newRow("disk_file_no_header") << "file_no_compression.txt" << QByteArray() << 200 << QByteArray();
        QTest::newRow("disk_file_identity") << "file_no_compression.txt" << QByteArrayLiteral("identity") << 200 << QByteArray();
        QTest::newRow("disk_file_identity_disabled") << "file_no_compression.txt" << QByteArrayLiteral("identity;q=0") << 406 << QByteArray();
        QTest::newRow("disk_file_deflate_only") << "file_no_compression.txt" << QByteArrayLiteral("deflate") << 200 << QByteArray();
        QTest::newRow("disk_file_wildcard") << "file_no_compression.txt" << QByteArrayLiteral("*") << 200 << QByteArray();
        QTest::newRow("disk_file_deflate_identity_disabled") << "file_no_compression.txt" << QByteArrayLiteral("deflate, identity;q=0") << 406 << QByteArray();
    */
    // Deflate resource
    QTest::newRow("deflate_resource_no_header") << "file_deflate.txt" << QByteArray() << 200 << QByteArrayLiteral("deflate");
    QTest::newRow("deflate_resource_deflate") << "file_deflate.txt" << QByteArrayLiteral("deflate") << 200 << QByteArrayLiteral("deflate");
    QTest::newRow("deflate_resource_identity") << "file_deflate.txt" << QByteArrayLiteral("identity") << 200 << QByteArray();
    QTest::newRow("deflate_resource_zstd") << "file_deflate.txt" << QByteArrayLiteral("zstd") << 200 << QByteArray();
    QTest::newRow("deflate_resource_identity_disabled") << "file_deflate.txt" << QByteArrayLiteral("identity;q=0") << 406 << QByteArray();

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    // Zstd resource
    QTest::newRow("zstd_resource_no_header") << "file_zstd.txt" << QByteArray() << 200 << QByteArray();
    QTest::newRow("zstd_resource_zstd") << "file_zstd.txt" << QByteArrayLiteral("zstd") << 200 << QByteArrayLiteral("zstd");
    QTest::newRow("zstd_resource_identity") << "file_zstd.txt" << QByteArrayLiteral("identity") << 200 << QByteArray();
    QTest::newRow("zstd_resource_deflate_only") << "file_zstd.txt" << QByteArrayLiteral("deflate") << 200 << QByteArray();
    QTest::newRow("zstd_resource_identity_disabled") << "file_zstd.txt" << QByteArrayLiteral("identity;q=0") << 200 << QByteArray();
#endif

    // 404 test
    QTest::newRow("not_found") << "nonexistent.txt" << QByteArray() << 404 << QByteArray();
}

void TestAcceptEncoding::encodingScenarios()
{
    QFETCH(QString, file);
    QFETCH(QByteArray, acceptEncoding);
    QFETCH(int, expectedStatus);
    QFETCH(QByteArray, expectedEncoding);

    QUrl url(QString("http://localhost:%1/%2").arg(server->serverPort()).arg(file));
    QNetworkRequest request(url);
    if (!acceptEncoding.isEmpty())
        request.setRawHeader("Accept-Encoding", acceptEncoding);

    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(3000));

    const int actualStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0) && !QT_CONFIG(zstd)
    QEXPECT_FAIL("zstd_resource_no_header", "Qt compiled without zstd support", Abort);
    QEXPECT_FAIL("zstd_resource_zstd", "Qt compiled without zstd support", Abort);
    QEXPECT_FAIL("zstd_resource_identity", "Qt compiled without zstd support", Abort);
    QEXPECT_FAIL("zstd_resource_deflate_only", "Qt compiled without zstd support", Abort);
    QEXPECT_FAIL("zstd_resource_identity_disabled", "Qt compiled without zstd support", Abort);
#endif
    QCOMPARE(actualStatus, expectedStatus);

    const QByteArray actualEncoding = reply->rawHeader("Content-Encoding");
    QCOMPARE(actualEncoding, expectedEncoding);

    // Unfortunately QtNetworkAccessManager turns off decompression for us if we ask for a specific encoding.
    // We can therefore only check if the data was received correctly if we use the default headers
    // or we implement our own decompression.
    if (expectedStatus == 200 && acceptEncoding.isEmpty()) {
        QFile f("searchpath:" + file);
        QVERIFY2(f.open(QIODevice::ReadOnly), qPrintable(QString("Could not open %1").arg(file)));
        const QByteArray expectedContent = f.readAll();
        QCOMPARE(reply->readAll(), expectedContent);
    }

    reply->deleteLater();
}

QTEST_MAIN(TestAcceptEncoding)
#include "test_accept_encoding.moc"
