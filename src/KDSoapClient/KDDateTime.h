/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDDATETIME_H
#define KDDATETIME_H

#include "KDSoapGlobal.h"

#include <QtCore/QDateTime>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QVariant>

class KDDateTimeData;

/**
 * A DateTime class with an additional (optional) timezone.
 *
 * As specified in https://www.w3.org/TR/xmlschema-2/#dateTime, the timezone
 * can be empty (local time), "Z" (for UTC) or an offset from UTC like "+05:00" or "-03:00"
 */
class KDSOAP_EXPORT KDDateTime : public QDateTime
{
public:
    KDDateTime();
    KDDateTime(const KDDateTime &);
    /**
     * Implicit constructor from a QDateTime.
     * Sets the timeZone to "local", i.e. empty
     */
    /*implicit*/ KDDateTime(const QDateTime &);
    KDDateTime &operator=(const KDDateTime &);
    ~KDDateTime();

    /**
     * Converts the KDDateTime to QVariant, to avoid implicit conversion
     * to base QDateTime.
     * \since 1.8
     */
    operator QVariant() const;

    /**
     * Returns the time zone set by setTimeZone.
     */
    QString timeZone() const;
    /**
     * Sets the timeZone.
     * Can be empty, "Z", or an offset like "+05:00" or "-03:00".
     */
    void setTimeZone(const QString &timeZone);

    /**
     * Creates a KDDateTime from a SOAP-compliant string representation.
     */
    static KDDateTime fromDateString(const QString &s);

    /**
     * Returns a SOAP-compliant string representation of the date/time object.
     */
    QString toDateString() const;

private:
    QSharedDataPointer<KDDateTimeData> d;
};

Q_DECLARE_METATYPE(KDDateTime)

#endif // KDDATETIME_H
