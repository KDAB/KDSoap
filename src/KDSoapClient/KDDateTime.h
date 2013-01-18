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
#ifndef KDDATETIME_H
#define KDDATETIME_H

#include "KDSoapGlobal.h"

#include <QtCore/QDateTime>
#include <QtCore/QVariant>
#include <QtCore/QSharedDataPointer>

class KDDateTimeData;

/**
 * A DateTime class with an additional (optional) timezone.
 *
 * As specified in http://www.w3.org/TR/xmlschema-2/#dateTime, the timezone
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
    KDDateTime(const QDateTime &);
    KDDateTime &operator=(const KDDateTime &);
    ~KDDateTime();

    /**
     * Returns the time zone set by setTimeZone.
     */
    QString timeZone() const;
    /**
     * Sets the timeZone.
     * Can be empty, "Z", or an offset like "+05:00" or "-03:00".
     */
    void setTimeZone(const QString& timeZone);

    /**
     * Creates a KDDateTime from a SOAP-compliant string representation.
     */
    static KDDateTime fromDateString(const QString& s);

    /**
     * Returns a SOAP-compliant string representation of the date/time object.
     */
    QString toDateString() const;

private:
    QSharedDataPointer<KDDateTimeData> d;
};

Q_DECLARE_METATYPE(KDDateTime)

#endif // KDDATETIME_H
