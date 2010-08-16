/*
    This file is part of KDE.

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

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QDebug>

#include "settings.h"

class SettingsSingleton
{
public:
    Settings mSettings;
};

Q_GLOBAL_STATIC(SettingsSingleton, s_settings)

Settings::Settings()
{
  mOutputDirectory = QDir::current().path();
  mOutputFileName = "kwsdl_generated";
  mImpl = false;
}

Settings::~Settings()
{
}

Settings* Settings::self()
{
    return &s_settings()->mSettings;
}

void Settings::setWsdlFile(const QString &wsdlFile)
{
    mWsdlFile = QDir::fromNativeSeparators(wsdlFile);

    if (QDir::isRelativePath(wsdlFile))
        mWsdlFile = QDir::current().path() + '/' + mWsdlFile;

    //qDebug() << this << "setWsdlUrl: remembering" << mWsdlFile;
}

QUrl Settings::wsdlUrl() const
{
    //qDebug() << this << "get:" << mWsdlFile;
    return QUrl::fromLocalFile(mWsdlFile);
}

QString Settings::wsdlBaseUrl() const
{
  return mWsdlFile.left( mWsdlFile.lastIndexOf( '/' ) );
}

QString Settings::wsdlFileName() const
{
  return mWsdlFile.mid( mWsdlFile.lastIndexOf( '/' ) + 1 );
}

void Settings::setOutputFileName( const QString &outputFileName )
{
  mOutputFileName = outputFileName;
}

QString Settings::outputFileName() const
{
    if (mOutputFileName.isEmpty()) {
        QFileInfo fi(wsdlFileName());
        return "wsdl_" + fi.completeBaseName() + (mImpl ? ".cpp" : ".h");
    }

    return mOutputFileName;
}

void Settings::setOutputDirectory( const QString &outputDirectory )
{
  mOutputDirectory = outputDirectory;

  if ( !mOutputDirectory.endsWith( "/" ) )
    mOutputDirectory.append( "/" );
}

QString Settings::outputDirectory() const
{
  return mOutputDirectory;
}

void Settings::setNamespaceMapping( const NSMapping &namespaceMapping )
{
  mNamespaceMapping = namespaceMapping;
}

Settings::NSMapping Settings::namespaceMapping() const
{
  return mNamespaceMapping;
}

void Settings::setGenerateImplementation(bool b, const QString& headerFile)
{
    mImpl = b;
    mHeaderFile = headerFile;
}

bool Settings::generateImplementation() const
{
    return mImpl;
}

QString Settings::headerFile() const
{
    return mHeaderFile;
}

void Settings::setWantedService(const QString &service)
{
    mWantedService = service;
}

QString Settings::wantedService() const
{
    return mWantedService;
}
