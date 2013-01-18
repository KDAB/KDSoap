/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#include "KDDateTime.h"
#include <QSharedData>
#include <QDebug>

class KDDateTimeData : public QSharedData {
public:
    QString mTimeZone;
};

KDDateTime::KDDateTime() : d(new KDDateTimeData)
{
}

KDDateTime::KDDateTime(const KDDateTime &rhs)
    : QDateTime(rhs), d(rhs.d)
{
}

KDDateTime::KDDateTime(const QDateTime &rhs)
    : QDateTime(rhs), d(new KDDateTimeData)
{
}

KDDateTime &KDDateTime::operator=(const KDDateTime &rhs)
{
    if (this != &rhs) {
        QDateTime::operator=(rhs);
        d.operator=(rhs.d);
    }
    return *this;
}

KDDateTime::~KDDateTime()
{
}

QString KDDateTime::timeZone() const
{
    return d->mTimeZone;
}

void KDDateTime::setTimeZone(const QString &timeZone)
{
    d->mTimeZone = timeZone;

    // Just in case someone cares: set the time spec in QDateTime accordingly.
    // We can't do this the other way round, there's no public API for the offset-from-utc case.
    if (timeZone == QLatin1String("Z"))
        setTimeSpec(Qt::UTC);
    else if (timeZone.isEmpty())
        setTimeSpec(Qt::LocalTime);
    else {
        setTimeSpec(Qt::OffsetFromUTC);
        const int pos = timeZone.indexOf(QLatin1Char(':'));
        if ( pos > 0 ) {
            const int hours = timeZone.left(pos).toInt();
            const int minutes = timeZone.mid(pos+1).toInt();
            const int offset = hours * 3600 + minutes * 60;
            setUtcOffset(offset);
        }
    }
}

KDDateTime KDDateTime::fromDateString(const QString &s)
{
    KDDateTime kdt;
    QString tz;
    QString baseString = s;
    if (s.endsWith(QLatin1Char('Z'))) {
        tz = QString::fromLatin1("Z");
        baseString.chop(1);
    } else {
        QString maybeTz = s.right(6);
        if (maybeTz.startsWith(QLatin1Char('+')) || maybeTz.startsWith(QLatin1Char('-'))) {
            tz = maybeTz;
            baseString.chop(6);
        }
    }
    //qDebug() << "KDDateTime: Parsing" << baseString << "tz=" << tz;
    kdt = QDateTime::fromString(baseString, Qt::ISODate);
    kdt.setTimeZone(tz);
    return kdt;
}

QString KDDateTime::toDateString() const
{
    QString str;
    if (time().msec()) {
        // include milli-seconds
        str = toString(QLatin1String("yyyy-MM-ddThh:mm:ss.zzz"));
        str += d->mTimeZone;
    } else {
        str = toString(Qt::ISODate);
#if QT_VERSION < 0x040800 // Qt adds the timezone since 4.8
        str += d->mTimeZone;
#endif
    }
    return str;
}
