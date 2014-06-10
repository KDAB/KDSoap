#ifndef TESTCALL_H
#define TESTCALL_H

#include <QObject>
#include "wsdl_test.h"

class TestCall : public QObject
{
    Q_OBJECT
public:
    explicit TestCall();

private slots:
    void test();

private:
     PREFIX test_client;
};

#endif // TESTCALL_H
