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
#include <QBuffer>
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
    explicit FileServerObject(const QByteArray &data)
        : m_data(data)
    {
    }
    ~FileServerObject() = default;

    QIODevice *processFileRequest(const QString &path, QByteArray &contentType) override
    {
        Q_ASSERT(!path.startsWith(".."));
        if (path == QLatin1String("/path/to/file_download.txt")) {
            contentType = "text/plain";
            return new QBuffer(new QByteArray(m_data)); // KDSoap will delete this
        }
        return nullptr;
    }

private:
    QByteArray m_data;
};

class FileServer final : public KDSoapServer
{
    Q_OBJECT
public:
    explicit FileServer(const QByteArray &data)
        : m_data(data)
    {
    }
    ~FileServer() = default;

    QObject *createServerObject() override
    {
        return new FileServerObject(m_data);
    }

private:
    QByteArray m_data;
};

class TestRangeRequests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test for various valid single ranges and suffix ranges
    void testValidRanges_data();
    void testValidRanges();

    // Test for multiple ranges and coalescing behavior
    void testMultipleRanges_data();
    void testMultipleRanges();

    // Test for invalid ranges, malformed headers, invalid units, and mixed invalid/valid
    void testInvalidRanges_data();
    void testInvalidRanges();

    // Test coalescing of adjacent ranges
    void testAdjacentRangesCoalesced();

    void testPartiallyUnsatisfiableRanges();

    void testUnsatisfiableRanges_data();
    void testUnsatisfiableRanges();

    void testRangeWithWhitespace_data();
    void testRangeWithWhitespace();

#if 0
    // Test HEAD request without Range header returns correct headers, no body
    void testHeadRequestReturnsHeaders();

    // Test HEAD request with Range header returns headers, no body, status 200 or 206
    void testHeadRequestWithRange();
#endif

private:
    QByteArray expectedRange(int start, int length);

    FileServer *server = nullptr;
    QNetworkAccessManager *manager = nullptr;
    QByteArray testFileData;
    QUrl testUrl;
};

QByteArray TestRangeRequests::expectedRange(int start, int length)
{
    return testFileData.mid(start, length);
}

void TestRangeRequests::initTestCase()
{
    testFileData.clear();
    for (int i = 0; i < 2048; ++i) {
        testFileData.append(static_cast<char>((i * 37 + 11) % 256));
    }

    server = new FileServer(testFileData);
    QVERIFY(server->listen(QHostAddress::LocalHost));

    testUrl = QUrl(QString("http://localhost:%1/path/to/file_download.txt").arg(server->serverPort()));

    manager = new QNetworkAccessManager(this);
}

void TestRangeRequests::cleanupTestCase()
{
    delete server;
    delete manager;
}

void TestRangeRequests::testValidRanges_data()
{
    QTest::addColumn<QString>("rangeHeader");
    QTest::addColumn<QList<QByteArray>>("expectedRanges");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("full_range") << "bytes=0-2047"
                                << QList<QByteArray> {expectedRange(0, 2048)} << 200; // Full file, 200 OK

    QTest::newRow("suffix_equal_file") << "bytes=-2048"
                                       << QList<QByteArray> {expectedRange(0, 2048)} << 200; // Full file suffix, 200 OK

    QTest::newRow("suffix_larger_than_file") << "bytes=-9999"
                                             << QList<QByteArray> {expectedRange(0, 2048)} << 200; // Suffix bigger than file, 200 OK

    QTest::newRow("valid_single_range") << "bytes=100-199"
                                        << QList<QByteArray> {expectedRange(100, 100)} << 206; // Partial content

    QTest::newRow("multiple_ranges") << "bytes=100-199,300-399"
                                     << QList<QByteArray> {expectedRange(100, 100), expectedRange(300, 100)} << 206; // Partial content

    QTest::newRow("adjacent_coalesced") << "bytes=100-150,151-160"
                                        << QList<QByteArray> {expectedRange(100, 61)} << 206; // Coalesced ranges

    QTest::newRow("invalid_range_order") << "bytes=200-100"
                                         << QList<QByteArray> {} << 416; // Unsatisfiable

    QTest::newRow("unsatisfiable_range") << "bytes=3000-4000"
                                         << QList<QByteArray> {} << 416; // Unsatisfiable
}

void TestRangeRequests::testValidRanges()
{
    QFETCH(QString, rangeHeader);
    QFETCH(QList<QByteArray>, expectedRanges);
    QFETCH(int, expectedStatusCode);

    QNetworkRequest request(testUrl);
    if (!rangeHeader.isEmpty())
        request.setRawHeader("Range", rangeHeader.toUtf8());

    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (expectedStatusCode != 200 || status == 200) // When transferring the whole file can either reply with 200 or 206
        QCOMPARE(status, expectedStatusCode);
    else
        QCOMPARE(status, 206);

    if (status == 206) {
        QByteArray data = reply->readAll();
        for (const QByteArray &range : expectedRanges) {
            QVERIFY(data.contains(range));
        }
    } else if (status == 200) {
        // For 200 OK full content, expect exact data match
        QCOMPARE(reply->readAll(), testFileData);
    } else {
        // For 416 or others, body should be empty
        QCOMPARE(reply->readAll().size(), 0);
    }

    reply->deleteLater();
}

void TestRangeRequests::testMultipleRanges_data()
{
    QTest::addColumn<QString>("rangeHeader");
    QTest::addColumn<QList<QByteArray>>("expectedRanges");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("separate_non-overlapping") << "bytes=100-199,300-399"
                                              << QList<QByteArray> {expectedRange(100, 100), expectedRange(300, 100)} << 206;

    QTest::newRow("adjacent_should_coalesce") << "bytes=400-499,500-599"
                                              << QList<QByteArray> {expectedRange(400, 200)} << 206;

    QTest::newRow("overlapping_should_coalesce") << "bytes=600-700,650-750"
                                                 << QList<QByteArray> {expectedRange(600, 151)} << 206;

    QTest::newRow("gapped_should_stay_separate") << "bytes=800-850,860-900"
                                                 << QList<QByteArray> {expectedRange(800, 51), expectedRange(860, 41)} << 206;

    // Range covers full file, status can be 200 or 206, let's pick 206 and accept both in test
    QTest::newRow("suffix_equal_to_file") << "bytes=-2048"
                                          << QList<QByteArray> {expectedRange(0, 2048)} << 200;

    QTest::newRow("unsatisfiable_range") << "bytes=3000-4000"
                                         << QList<QByteArray> {} << 416;

    QTest::newRow("separate_non-overlapping_includes_start") << "bytes=0-100,200-299"
                                                             << QList<QByteArray> {expectedRange(0, 100), expectedRange(200, 100)} << 206;
}

void TestRangeRequests::testMultipleRanges()
{
    QFETCH(QString, rangeHeader);
    QFETCH(QList<QByteArray>, expectedRanges);
    QFETCH(int, expectedStatusCode);

    QNetworkRequest request(testUrl);
    request.setRawHeader("Range", rangeHeader.toUtf8());

    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (expectedRanges.isEmpty()) {
        QCOMPARE(status, 416);
        QCOMPARE(status, expectedStatusCode);
    } else if (expectedRanges.size() == 1) {
        if (expectedStatusCode == 200 && status != 200) {
            // Accept 200 or 206 when full content is returned for request
            QCOMPARE(status, 206);
        } else
            QCOMPARE(status, expectedStatusCode);
        const QByteArray contentTypeHeader = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
        QVERIFY(!contentTypeHeader.startsWith("multipart/byteranges"));
        const QByteArray data = reply->readAll();
        QCOMPARE(data, expectedRanges.first());
    } else {
        QCOMPARE(status, 206);
        QCOMPARE(status, expectedStatusCode);
        const QByteArray contentTypeHeader = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
        QVERIFY(contentTypeHeader.startsWith("multipart/byteranges; boundary="));
        const QByteArray boundary = contentTypeHeader.split('=').last();
        const QByteArray body = reply->readAll();

        int pos = 0;
        for (int i = 0; i < expectedRanges.size(); ++i) {
            const QByteArray boundaryMarker = "--" + boundary + "\r\n";
            const int boundaryIndex = body.indexOf(boundaryMarker, pos);
            QVERIFY(boundaryIndex >= 0);
            pos = boundaryIndex + boundaryMarker.length();

            const int headerEnd = body.indexOf("\r\n\r\n", pos);
            QVERIFY(headerEnd > pos);

            const QByteArray headers = body.mid(pos, headerEnd - pos);
            QVERIFY(headers.contains("Content-Range: bytes "));
            QVERIFY(headers.contains("Content-Type: text/plain"));

            pos = headerEnd + 4;

            const int expectedLen = expectedRanges[i].size();
            const QByteArray actual = body.mid(pos, expectedLen);
            QCOMPARE(actual, expectedRanges[i]);
            pos += expectedLen;
        }

        const QByteArray closingBoundary = "\r\n--" + boundary + "--\r\n";
        QVERIFY(body.mid(pos).startsWith(closingBoundary));
    }

    reply->deleteLater();
}

void TestRangeRequests::testInvalidRanges_data()
{
    QTest::addColumn<QString>("rangeHeader");
    QTest::addColumn<int>("expectedStatus");

    // Invalid byte order (last-byte-pos < first-byte-pos)
    QTest::newRow("invalid_range_order") << "bytes=200-100" << 416;

    // Malformed range header (non-numeric)
    QTest::newRow("malformed_range") << "bytes=abc-def" << 200;

    // Invalid unit (not 'bytes')
    QTest::newRow("invalid_range_unit") << "items=100-200" << 200;

    // Mixed valid and invalid range headers - malformed ranges cause the entire header to be ignored
    QTest::newRow("mixed_valid_invalid") << "bytes=100-200,invalid,300-400" << 200;

    // Too many ranges (more than 5)
    QTest::newRow("too_many_ranges") << "bytes=0-1,2-3,4-5,6-7,8-9,10-11" << 416;
}

void TestRangeRequests::testInvalidRanges()
{
    QFETCH(QString, rangeHeader);
    QFETCH(int, expectedStatus);

    QNetworkRequest request(testUrl);
    request.setRawHeader("Range", rangeHeader.toUtf8());

    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QCOMPARE(status, expectedStatus);

    reply->deleteLater();
}

void TestRangeRequests::testPartiallyUnsatisfiableRanges()
{
    QNetworkRequest request(testUrl);
    // First range is valid, second range is beyond EOF
    request.setRawHeader("Range", "bytes=0-99,99999-100000");
    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(status, 206); // Only valid part should be returned

    const QByteArray data = reply->readAll();
    QCOMPARE(data, expectedRange(0, 100));

    reply->deleteLater();
}

void TestRangeRequests::testUnsatisfiableRanges_data()
{
    QTest::addColumn<QString>("rangeHeader");

    QTest::newRow("too_many_ranges") << "bytes=0-1,2-3,4-5,6-7,8-9,10-11";
    QTest::newRow("start_beyond_eof") << "bytes=99999-100000";
    QTest::newRow("all_unsatisfiable_multiple") << "bytes=99999-100000,3000-4000";
}

void TestRangeRequests::testUnsatisfiableRanges()
{
    QFETCH(QString, rangeHeader);

    QNetworkRequest request(testUrl);
    request.setRawHeader("Range", rangeHeader.toUtf8());
    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 416);
    reply->deleteLater();
}

void TestRangeRequests::testRangeWithWhitespace_data()
{
    QTest::addColumn<QString>("rangeHeader");
    QTest::addColumn<QList<QByteArray>>("expectedRanges");

    QTest::newRow("spaces_around_ranges_and_commas")
        << "bytes= 0 - 99 ,  200 - 299 "
        << QList<QByteArray> {expectedRange(0, 100), expectedRange(200, 100)};

    QTest::newRow("tabs_around_ranges_and_commas")
        << "bytes=\t0-49,\t50-99\t"
        << QList<QByteArray> {expectedRange(0, 50), expectedRange(50, 50)};

    QTest::newRow("mixed_spaces_and_tabs")
        << "bytes=  0-9 ,\t 10 -19 , 20 - 29\t"
        << QList<QByteArray> {expectedRange(0, 10), expectedRange(10, 10), expectedRange(20, 10)};
}

void TestRangeRequests::testRangeWithWhitespace()
{
    QFETCH(QString, rangeHeader);
    QFETCH(QList<QByteArray>, expectedRanges);

    QNetworkRequest request(testUrl);
    request.setRawHeader("Range", rangeHeader.toUtf8());
    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

    if (expectedRanges.isEmpty()) {
        // No valid ranges - expect 416
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 416);
    } else if (expectedRanges.size() == 1) {
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 206);
        const QByteArray body = reply->readAll();
        QCOMPARE(body, expectedRanges[0]);
    } else {
        // Multiple ranges: expect multipart/byteranges
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 206);
        const QByteArray body = reply->readAll();

        for (const QByteArray &rangeData : expectedRanges) {
            QVERIFY(body.contains(rangeData));
        }
    }
    reply->deleteLater();
}

void TestRangeRequests::testAdjacentRangesCoalesced()
{
    // Test that adjacent ranges in one header are coalesced into a single range response
    QNetworkRequest request(testUrl);
    request.setRawHeader("Range", "bytes=100-199,200-299");

    QNetworkReply *reply = manager->get(request);
    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 206);

    QByteArray content = reply->readAll();

    // Should contain the combined range from 100-299 (length 200)
    QCOMPARE(content, expectedRange(100, 200));

    reply->deleteLater();
}

#if 0
void TestRangeRequests::testHeadRequestReturnsHeaders()
{
    QNetworkRequest request(testUrl);
    QNetworkReply *reply = manager->head(request);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

           // HEAD should return 200 OK (full file) with Content-Length but no body
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);

    QVariant contentLength = reply->header(QNetworkRequest::ContentLengthHeader);
    QVERIFY(contentLength.isValid());
    QCOMPARE(contentLength.toInt(), testFileData.size());

           // Should have no body on HEAD
    QCOMPARE(reply->readAll().size(), 0);

    reply->deleteLater();
}

void TestRangeRequests::testHeadRequestWithRange()
{
    QNetworkRequest request(testUrl);
    request.setRawHeader("Range", "bytes=0-99");

    QNetworkReply *reply = manager->head(request);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait());

           // Should return 206 Partial Content with Content-Length 100 but no body on HEAD
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QVERIFY(status == 206 || status == 200);

    QVariant contentLength = reply->header(QNetworkRequest::ContentLengthHeader);
    QVERIFY(contentLength.isValid());
    QCOMPARE(contentLength.toInt(), 100);

           // Should have no body on HEAD
    QCOMPARE(reply->readAll().size(), 0);

    reply->deleteLater();
}
#endif

QTEST_MAIN(TestRangeRequests)
#include "test_ranges.moc"
