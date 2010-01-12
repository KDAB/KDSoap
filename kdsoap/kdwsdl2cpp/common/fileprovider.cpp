/*
    This file is part of KDE Schema Parser

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

#include <unistd.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QUrl>
#include <QDebug>

#include "fileprovider.h"

FileProvider::FileProvider()
  : QObject( 0 ), mBlocked( false )
{
}

bool FileProvider::get( const QUrl &url, QString &target )
{
  if ( !mFileName.isEmpty() )
    cleanUp();

  if (url.scheme() == QLatin1String("file")) {
      target = url.toLocalFile();
      return true;
  }

  if ( target.isEmpty() ) {
#ifdef KDAB_TEMP
    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    tmpFile.open();
    target = tmpFile.fileName();
    mFileName = target;
#endif
  }

  mData.truncate( 0 );

  qDebug("NOT IMPLEMENTED: downloading external schema '%s'", url.toEncoded().constData());
#ifdef KDAB_TEMP

  KIO::TransferJob* job = KIO::get( KUrl( url ), KIO::NoReload, KIO::HideProgressInfo );
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
           this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
  connect( job, SIGNAL( result( KJob* ) ),
           this, SLOT( slotResult( KJob* ) ) );

  mBlocked = true;
  while ( mBlocked ) {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    usleep( 500 );
  }
#endif

  return true;
}

void FileProvider::cleanUp()
{
  ::unlink( QFile::encodeName( mFileName ) );
  mFileName = QString();
}

#ifdef KDAB_TEMP
void FileProvider::slotData( KIO::Job*, const QByteArray &data )
{
  unsigned int oldSize = mData.size();
  mData.resize( oldSize + data.size() );
  memcpy( mData.data() + oldSize, data.data(), data.size() );
}

void FileProvider::slotResult( KJob *job )
{
  if ( job->error() ) {
    qDebug( "%s", qPrintable( job->errorText() ) );
    mBlocked = false;
    return;
  }

  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    qDebug( "Unable to create temporary file" );
    mBlocked = false;
    return;
  }

  qDebug( "Download successful" );
  file.write( mData );
  file.close();

  mData.truncate( 0 );

  mBlocked = false;
}
#endif

#include "moc_fileprovider.cpp"
