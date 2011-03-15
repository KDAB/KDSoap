#ifndef KDDATETIME_H
#define KDDATETIME_H

#include "KDSoapGlobal.h"

#include <QDateTime>
#include <QVariant>
#include <QSharedDataPointer>

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
