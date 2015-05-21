#include <QString>
#include <QThread>
#include <QtTest>

#include <airtunes/airtunesconstants.h>
#include <rtp/rtpbuffer.h>
#include <rtp/rtppacket.h>

const uint numPacketsPerSecond = 44100/airtunes::framesPerPacket;
const quint16 initSeqNo = 65000;    // we want to wrap the buffer

class Producer : public QThread
{
    Producer(RtpBuffer *rtpBuffer) :
        m_rtpBuffer(rtpBuffer)
    {
    }

public:
    void run()
    {
        std::srand(std::time(0));
        for (quint16 i = initSeqNo; i < initSeqNo+(numPacketsPerSecond*30); ++i) {
            RtpPacket *rtpPacket = m_rtpBuffer->obtainPacket(i);
            for (uint j = 0; j < airtunes::framesPerPacket; ++j) {
                (int*)(rtpPacket->payload+j) = std::rand();
            }
        }
    }

private:
    RtpBuffer *m_rtpBuffer;
};


class RtpTest : public QObject
{
    Q_OBJECT

public:
    RtpTest();

private Q_SLOTS:
    void regular_data();
    void regular();
    //void highJitter();
    //void singleLoss();
    //void burstLoss();
};

RtpTest::RtpTest()
{
}

void RtpTest::regular_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

//void RtpTest::regular()
//{
//    QFETCH(QString, data);
//    QVERIFY2(true, "Failure");
//}

void RtpTest::regular()
{
    RtpBuffer *rtpBuffer = new RtpBuffer(airtunes::framesPerPacket, 500);
    //rtpBuffer->obtainPacket()

}

QTEST_APPLESS_MAIN(RtpTest)

#include "tst_rtptest.moc"
