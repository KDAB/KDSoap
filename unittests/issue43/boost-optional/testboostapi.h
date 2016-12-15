#ifndef TESTBOOSTAPI_H
#define TESTBOOSTAPI_H

#include <QObject>
#include "wsdl_test.h"

class TestBoostApi : public QObject
{
    Q_OBJECT
public:
    explicit TestBoostApi();

private slots:
    void test();

private:
};

#endif // TESTBOOSTAPI_H
