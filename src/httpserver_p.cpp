#include "httpserver_p.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QEventLoop>

// Helper for xmlBufferCompare
static bool textBufferCompare(
    const QByteArray& source, const QByteArray& dest,  // for the qDebug only
    QIODevice& sourceFile, QIODevice& destFile)
{
    int lineNumber = 1;
    while (!sourceFile.atEnd()) {
        if (destFile.atEnd())
            return false;
        QByteArray sourceLine = sourceFile.readLine();
        QByteArray destLine = destFile.readLine();
        if (sourceLine != destLine) {
            sourceLine.chop(1); // remove '\n'
            destLine.chop(1); // remove '\n'
            qDebug() << source << "and" << dest << "differ at line" << lineNumber;
            qDebug("got     : %s", sourceLine.constData());
            qDebug("expected: %s", destLine.constData());
            return false;
        }
        ++lineNumber;
    }
    return true;
}

// A tool for comparing XML documents and outputting something useful if they differ
bool KDSoapUnitTestHelpers::xmlBufferCompare(const QByteArray& source, const QByteArray& dest)
{
    QBuffer sourceFile;
    sourceFile.setData(source);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR opening QIODevice";
        return false;
    }
    QBuffer destFile;
    destFile.setData(dest);
    if (!destFile.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR opening QIODevice";
        return false;
    }

    // Use QDomDocument to reformat the XML with newlines
    QDomDocument sourceDoc;
    if (!sourceDoc.setContent(&sourceFile)) {
        qDebug() << "ERROR parsing XML:" << source;
        return false;
    }
    QDomDocument destDoc;
    if (!destDoc.setContent(&destFile)) {
        qDebug() << "ERROR parsing XML:" << dest;
        return false;
    }

    const QByteArray sourceXml = sourceDoc.toByteArray();
    const QByteArray destXml = destDoc.toByteArray();
    sourceFile.close();
    destFile.close();

    QBuffer sourceBuffer;
    sourceBuffer.setData(sourceXml);
    sourceBuffer.open(QIODevice::ReadOnly);
    QBuffer destBuffer;
    destBuffer.setData(destXml);
    destBuffer.open(QIODevice::ReadOnly);

    return textBufferCompare(source, dest, sourceBuffer, destBuffer);
}

void KDSoapUnitTestHelpers::httpGet(const QUrl& url)
{
    QNetworkRequest request(url);
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.get(request);
    //reply->ignoreSslErrors();

    QEventLoop ev;
    QObject::connect(reply, SIGNAL(finished()), &ev, SLOT(quit()));
    ev.exec();

    //QObject::connect(reply, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    //QTestEventLoop::instance().enterLoop(11);

    delete reply;
}

QByteArray KDSoapUnitTestHelpers::makeHttpResponse(const QByteArray& responseData)
{
    QByteArray httpResponse("HTTP/1.0 200 OK\r\nContent-Type: text/xml\r\nContent-Length: ");
    httpResponse += QByteArray::number(responseData.size());
    httpResponse += "\r\n\r\n";
    httpResponse += responseData;
    return httpResponse;
}
