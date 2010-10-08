#include "KDSoapValue.h"
#include <QtTest/QtTest>

class Basic : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testValueSwap()
    {
        static const QLatin1String hello("Hello, World!" );
        KDSoapValue v1( QLatin1String( "v1" ), 10 );
        KDSoapValue v2( QLatin1String( "v2" ), hello );
        v1.swap( v2 );
        QCOMPARE( v1.value().toString(), hello );
        QCOMPARE( v1.name(), QLatin1String( "v2" ) );
        QCOMPARE( v2.value().toInt(), 10 );
        QCOMPARE( v2.name(), QLatin1String( "v1" ) );
        qSwap( v1, v2 );
        QCOMPARE( v1.value().toInt(), 10 );
        QCOMPARE( v2.value().toString(), hello );
#ifndef QT_NO_STL
        using std::swap;
        swap( v2, v1 );
        QCOMPARE( v1.value().toString(), hello );
        QCOMPARE( v2.value().toInt(), 10 );
#endif
    }
};

QTEST_MAIN(Basic)

#include "basic.moc"

