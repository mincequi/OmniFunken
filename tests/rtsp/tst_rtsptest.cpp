#include <QString>
#include <QtTest>
#include <QCoreApplication>

class RtspTest : public QObject
{
    Q_OBJECT

public:
    RtspTest();

private Q_SLOTS:
    void testCase1();
};

RtspTest::RtspTest()
{
}

void RtspTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_MAIN(RtspTest)

#include "tst_rtsptest.moc"
