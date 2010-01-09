#ifndef SOAPRESPONDER_H
#define SOAPRESPONDER_H

#include <QObject>
class KDSoapClientInterface;
class KDSoapMessage;

class SoapResponder : public QObject
{
    Q_OBJECT
public:
    explicit SoapResponder(QObject *parent = 0);
    virtual ~SoapResponder();

    /// Return the error from the last blocking call.
    /// Empty if no error
    QString lastError() const;

    /// Blocking call to Method1. Not recommended in a GUI thread.
    QString Method1(const QString& bstrParam1, const QString& bstrParam2);

public:
    /// Asynchronous call to Method1.
    /// Remember to connect to Method1Done and Method1Error.
    void asyncMethod1(const QString& bstrParam1, const QString& bstrParam2);
Q_SIGNALS:
    void Method1Done(const QString& returnValue);
    void Method1Error(const KDSoapMessage& fault);

private:
    Q_PRIVATE_SLOT(d, void _kd_slotMethod1Finished(KDSoapPendingCallWatcher*))

    class Private;
    Private* d;
};

#endif // SOAPRESPONDER_H
