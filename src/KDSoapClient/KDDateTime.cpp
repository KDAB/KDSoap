/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDDateTime.h"
#include <QDebug>
#include <QSharedData>

class KDDateTimeData : public QSharedData
{
public:
    QString mTimeZone;
};

KDDateTime::KDDateTime()
    : d(new KDDateTimeData)
{
}

KDDateTime::KDDateTime(const KDDateTime &rhs)
    : QDateTime(rhs)
    , d(rhs.d)
{
}

KDDateTime::KDDateTime(const QDateTime &rhs)
    : QDateTime(rhs)
    , d(new KDDateTimeData)
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

KDDateTime::operator QVariant() const
{
    return QVariant::fromValue(*this);
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
    if (timeZone == QLatin1String("Z")) {
        setTimeSpec(Qt::UTC);
    } else if (timeZone.isEmpty()) {
        setTimeSpec(Qt::LocalTime);
    } else {
        setTimeSpec(Qt::OffsetFromUTC);
        const int pos = timeZone.indexOf(QLatin1Char(':'));
        if (pos > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            const int hours = timeZone.leftRef(pos).toInt();
            const int minutes = timeZone.midRef(pos + 1).toInt();
#else
            const QStringView timeZoneView(timeZone);
            const int hours = timeZoneView.first(pos).toInt();
            const int minutes = timeZoneView.sliced(pos + 1).toInt();
#endif
            const int offset = hours * 3600 + minutes * 60;
            setOffsetFromUtc(offset);
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
    // qDebug() << "KDDateTime: Parsing" << baseString << "tz=" << tz;
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
        str = toString(Qt::ISODate); // includes the timezone (since Qt 4.8)
    }
    return str;
}
