#ifndef KDSOAPMESSAGE_P_H
#define KDSOAPMESSAGE_P_H

#include <QString>
#include <QVariant>
#include <QList>

struct KDSoapValue
{
    KDSoapValue(const QString& n, const QVariant& v)
        : name(n), value(v) {}

    QString name;
    QVariant value;
};

typedef QList<KDSoapValue> KDSoapValueList;
typedef QListIterator<KDSoapValue> KDSoapValueListIterator;

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapValueList args;
};

#endif // KDSOAPMESSAGE_P_H
