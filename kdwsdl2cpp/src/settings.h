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
#include <QString>
#include <QUrl>

class Settings
{
public:
    typedef QMap<QString, QString> NSMapping;

    enum OptionalElementType { ENone, ERawPointer, EBoostOptional };

    ~Settings();

    static Settings *self();

    void setGenerateImplementation(bool b, const QString &headerFile);
    bool generateImplementation() const;
    QString headerFile() const;

    void setGenerateServerCode(bool b);
    bool generateServerCode() const;

    void setWsdlFile(const QString &wsdlFile);
    QUrl wsdlUrl() const;
    QString wsdlBaseUrl() const;
    QString wsdlFileName() const;

    void setOutputFileName(const QString &outputFileName);
    QString outputFileName() const;

    void setOutputDirectory(const QString &outputDirectory);
    QString outputDirectory() const;

    void setOptionalElementType(OptionalElementType optionalElementType);
    OptionalElementType optionalElementType() const;

    // UNUSED
    void setNamespaceMapping(const NSMapping &namespaceMapping);
    NSMapping namespaceMapping() const;

    void setWantedService(const QString &service);
    QString wantedService() const;

    QString exportDeclaration() const;
    void setExportDeclaration(const QString &exportDeclaration);

    QString nameSpace() const;
    void setNameSpace(const QString &ns);

private:
    friend class SettingsSingleton;
    Settings();

    QUrl mWsdlUrl;
    QString mOutputFileName;
    QString mOutputDirectory;
    QString mHeaderFile;
    QString mWantedService;
    QString mExportDeclaration;
    QString mNameSpace;
    NSMapping mNamespaceMapping;
    bool mImpl;
    bool mServer;
    OptionalElementType mOptionalElementType;
};

#endif
