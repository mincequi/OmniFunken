#include <QString>
#include <QtTest>

class CoreTest : public QObject
{
    Q_OBJECT

public:
    CoreTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void parseCommandLine_data();
    void parseCommandLine();
};

CoreTest::CoreTest()
{
}

void CoreTest::initTestCase()
{
}

void CoreTest::cleanupTestCase()
{
}

void CoreTest::parseCommandLine_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void CoreTest::parseCommandLine()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(CoreTest)

#include "tst_coretest.moc"
