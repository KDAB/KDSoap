#ifndef TESTPOINTERAPI_H
#define TESTPOINTERAPI_H

#include <QObject>
#include "wsdl_test.h"

class TestPointerApi : public QObject
{
    Q_OBJECT
public:
    explicit TestPointerApi();

private slots:
    void test();

private:
};

#endif // TESTPOINTERAPI_H
