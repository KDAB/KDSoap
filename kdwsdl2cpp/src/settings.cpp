/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#define QT_NO_CAST_TO_ASCII
#define QT_NO_CAST_FROM_ASCII

#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>

#include "settings.h"

class SettingsSingleton
{
public:
    Settings mSettings;
};

Q_GLOBAL_STATIC(SettingsSingleton, s_settings)

Settings::Settings()
    : mOutputDirectory(QDir::current().path())
{
}

bool Settings::skipAsyncJobs() const
{
    return mSkipAsyncJobs;
}

void Settings::setSkipAsyncJobs(bool skipAsyncJobs)
{
    mSkipAsyncJobs = skipAsyncJobs;
}

bool Settings::skipAsync() const
{
    return mSkipAsync;
}

void Settings::setSkipAsync(bool skipAsync)
{
    mSkipAsync = skipAsync;
}

bool Settings::skipSync() const
{
    return mSkipSync;
}

void Settings::setSkipSync(bool skipSync)
{
    mSkipSync = skipSync;
}

bool Settings::useLocalFilesOnly() const
{
    return mUseLocalFilesOnly;
}

void Settings::setUseLocalFilesOnly(bool useLocalFilesOnly)
{
    mUseLocalFilesOnly = useLocalFilesOnly;
}

Settings::~Settings()
{
}

Settings *Settings::self()
{
    return &s_settings()->mSettings;
}

void Settings::setWsdlFile(const QString &wsdlFile)
{
    QString path = QDir::fromNativeSeparators(wsdlFile);

    // Check first for files, since on Windows drive letters can be interpreted as schemes
    if (QDir::isAbsolutePath(path)) {
        mWsdlUrl = QUrl::fromLocalFile(path);
        return;
    }

    QUrl u(path);
    if (u.isRelative()) { // no scheme yet in the URL
        path = QDir::current().path() + QLatin1Char('/') + path;
        mWsdlUrl = QUrl::fromLocalFile(path);
    } else {
        mWsdlUrl = std::move(u);
    }
}

QUrl Settings::wsdlUrl() const
{
    return mWsdlUrl;
}

QString Settings::wsdlBaseUrl() const
{
    const QString strUrl = mWsdlUrl.toString();
    return strUrl.left(strUrl.lastIndexOf(QLatin1Char('/')));
}

QString Settings::wsdlFileName() const
{
    const QString strUrl = mWsdlUrl.toString();
    return strUrl.mid(strUrl.lastIndexOf(QLatin1Char('/')) + 1);
}

void Settings::setHeaderFileName(const QString &headerFileName)
{
    mHeaderFileName = headerFileName;
}

void Settings::setImplementationFileName(const QString &implementationFileName)
{
    mImplementationFileName = implementationFileName;
}

void Settings::setOutputDirectory(const QString &outputDirectory)
{
    mOutputDirectory = outputDirectory;

    if (!mOutputDirectory.endsWith(QLatin1Char('/'))) {
        mOutputDirectory.append(QLatin1Char('/'));
    }
}

QString Settings::outputDirectory() const
{
    return mOutputDirectory;
}

void Settings::setOptionalElementType(Settings::OptionalElementType optionalElementType)
{
    mOptionalElementType = optionalElementType;
}

Settings::OptionalElementType Settings::optionalElementType() const
{
    return mOptionalElementType;
}

void Settings::setKeepUnusedTypes(bool b)
{
    mKeepUnusedTypes = b;
}

bool Settings::keepUnusedTypes() const
{
    return mKeepUnusedTypes;
}

void Settings::setNamespaceMapping(const NSMapping &namespaceMapping)
{
    mNamespaceMapping = namespaceMapping;
}

Settings::NSMapping Settings::namespaceMapping() const
{
    return mNamespaceMapping;
}

void Settings::setGenerateImplementation(bool b)
{
    mImpl = b;
}

bool Settings::generateImplementation() const
{
    return mImpl;
}

void Settings::setGenerateHeader(bool b)
{
    mHeader = b;
}

bool Settings::generateHeader() const
{
    return mHeader;
}

QString Settings::headerFileName() const
{
    return mHeaderFileName;
}

QString Settings::implementationFileName() const
{
    return mImplementationFileName;
}


void Settings::setWantedService(const QString &service)
{
    mWantedService = service;
}

QString Settings::wantedService() const
{
    return mWantedService;
}

void Settings::setGenerateServerCode(bool b)
{
    mServer = b;
}

bool Settings::generateServerCode() const
{
    return mServer;
}

QString Settings::exportDeclaration() const
{
    return mExportDeclaration;
}

void Settings::setExportDeclaration(const QString &exportDeclaration)
{
    mExportDeclaration = exportDeclaration;
}

QString Settings::nameSpace() const
{
    return mNameSpace;
}

void Settings::setNameSpace(const QString &ns)
{
    mNameSpace = ns;
}

QStringList Settings::importPathList() const
{
    return mImportPathList;
}

void Settings::setImportPathList(const QStringList &importPathList)
{
    mImportPathList = importPathList;
    if (mWsdlUrl.isLocalFile()) {
        QFileInfo wsdlFileInfo(mWsdlUrl.toLocalFile());
        QString wsdlDirPath = wsdlFileInfo.absolutePath();
        mImportPathList.prepend(wsdlDirPath);
    }
}

void Settings::setHelpOnMissing(bool b)
{
    mHelpOnMissing = b;
}

bool Settings::helpOnMissing() const
{
    return mHelpOnMissing;
}
