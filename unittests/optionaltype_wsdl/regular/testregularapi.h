#ifndef TESTREGULARAPI_H
#define TESTREGULARAPI_H

#include <QObject>
#include "wsdl_test.h"

class TestRegularApi : public QObject
{
    Q_OBJECT
public:
    explicit TestRegularApi();

private slots:
    void test();

private:
};

#endif // TESTREGULARAPI_H
