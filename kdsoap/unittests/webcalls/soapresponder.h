#ifndef SOAPRESPONDER_H
#define SOAPRESPONDER_H

#include <QObject>
class KDSoapClientInterface;

class SoapResponder : public QObject
{
    Q_OBJECT
public:
    explicit SoapResponder(QObject *parent = 0);
    virtual ~SoapResponder();

    // Blocking call to Method1
    QString Method1(const QString& bstrParam1, const QString& bstrParam2);

    QString lastError() const;

    // Asynchronous call to Method1 - connect to Method1Done and Method1Error
    void asyncMethod1();

signals:

public slots:

private:
    class Private;
    Private* d;
};

#endif // SOAPRESPONDER_H
