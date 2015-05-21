#include <QString>
#include <QtTest>

class RtpTest : public QObject
{
    Q_OBJECT

public:
    RtpTest();

private Q_SLOTS:
    void regularStream_data();
    void regularStream();
};

RtpTest::RtpTest()
{
}

void RtpTest::regularStream_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void RtpTest::regularStream()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(RtpTest)

#include "tst_rtptest.moc"
