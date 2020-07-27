/*
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QStringList>
#include <QUrl>

class Settings
{
public:
    typedef QMap<QString, QString> NSMapping;

    enum OptionalElementType { ENone, ERawPointer, EBoostOptional, EStdOptional };

    ~Settings();

    static Settings *self();

    void setImplementationFileName(const QString &implFileName);
    void setHeaderFileName(const QString &implFileName);
    QString headerFileName() const;
    QString implementationFileName() const;

    void setGenerateImplementation(bool b);
    bool generateImplementation() const;

    void setGenerateHeader(bool b);
    bool generateHeader() const;

    void setGenerateServerCode(bool b);
    bool generateServerCode() const;

    void setWsdlFile(const QString &wsdlFile);
    QUrl wsdlUrl() const;
    QString wsdlBaseUrl() const;
    QString wsdlFileName() const;

    void setOutputDirectory(const QString &outputDirectory);
    QString outputDirectory() const;

    void setOptionalElementType(OptionalElementType optionalElementType);
    OptionalElementType optionalElementType() const;

    void setKeepUnusedTypes(bool b);
    bool keepUnusedTypes() const;

    // UNUSED
    void setNamespaceMapping(const NSMapping &namespaceMapping);
    NSMapping namespaceMapping() const;

    void setWantedService(const QString &service);
    QString wantedService() const;

    QString exportDeclaration() const;
    void setExportDeclaration(const QString &exportDeclaration);

    QString nameSpace() const;
    void setNameSpace(const QString &ns);

    QStringList importPathList() const;
    void setImportPathList(const QStringList &importPathList);

    bool useLocalFilesOnly() const;
    void setUseLocalFilesOnly(bool useLocalFilesOnly);

    bool helpOnMissing() const;
    void setHelpOnMissing(bool b);

    bool skipSync() const;
    void setSkipSync(bool skipSync);

    bool skipAsync() const;
    void setSkipAsync(bool skipAsync);

    bool skipAsyncJobs() const;
    void setSkipAsyncJobs(bool skipAsyncJobs);

private:
    friend class SettingsSingleton;
    Settings();

    QUrl mWsdlUrl;
    QString mOutputDirectory;
    QString mHeaderFileName;
    QString mImplementationFileName;
    QString mWantedService;
    QString mExportDeclaration;
    QString mNameSpace;
    QStringList mImportPathList;
    NSMapping mNamespaceMapping;
    OptionalElementType mOptionalElementType;
    bool mHeader = false;
    bool mImpl = false;
    bool mServer = false;
    bool mKeepUnusedTypes = false;
    bool mUseLocalFilesOnly = false;
    bool mHelpOnMissing = false;
    bool mSkipSync = false;
    bool mSkipAsync = false;
    bool mSkipAsyncJobs = false;
};

#endif
