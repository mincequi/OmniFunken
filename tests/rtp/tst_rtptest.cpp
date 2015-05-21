#include <QString>
#include <QThread>
#include <QtTest>

#include <audioout/audioout_ao.h>
#include <airtunes/airtunesconstants.h>
#include <rtp/rtpbufferalt.h>
#include <rtp/rtppacket.h>

const uint numPacketsPerSecond = 44100/airtunes::framesPerPacket;
const quint16 initSeqNo = 65000;    // we want to wrap the buffer

class Producer : public QThread
{
public:
    Producer(RtpBuffer *rtpBuffer) :
        m_rtpBuffer(rtpBuffer)
    {
    }

private:
    void run()
    {
        std::srand(std::time(0));
        for (quint16 i = initSeqNo; i != (quint16)(initSeqNo+(numPacketsPerSecond*30)); ++i) {
            QThread::msleep(7);
            RtpPacket *rtpPacket = m_rtpBuffer->obtainPacket(i);
            for (uint j = 0; j < airtunes::framesPerPacket; ++j) {
                *(((int*)(rtpPacket->payload)+j)) = std::rand();
            }
            rtpPacket->payloadSize = airtunes::framesPerPacket*4;
            rtpPacket->sequenceNumber = i;
            m_rtpBuffer->commitPacket(rtpPacket);
        }
    }

    RtpBuffer *m_rtpBuffer;
};

class Consumer : public QThread
{
public:
    Consumer(RtpBuffer *rtpBuffer) :
        m_rtpBuffer(rtpBuffer)
    {
        m_audioOut = new AudioOutAo();
        QSettings::SettingsMap settings;
        m_audioOut->init(settings);
    }
    ~Consumer()
    {
        m_audioOut->deinit();
        delete m_audioOut;
    }

private:
    void run()
    {
        m_audioOut->start();
        m_rtpBuffer->waitUntilReady();
        while (true) {
            const RtpPacket *packet = m_rtpBuffer->takePacket();
            if (!packet) {
                qWarning()<<Q_FUNC_INFO<< "no packet from buffer. Stopping playback.";
                break;
            }
            m_audioOut->play(packet->payload, packet->payloadSize);
        } // while
        m_audioOut->stop();
    }

    RtpBuffer   *m_rtpBuffer;
    AudioOutAo  *m_audioOut;
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
    RtpBuffer rtpBuffer(airtunes::framesPerPacket, 2000);
    Producer producer(&rtpBuffer);
    Consumer consumer(&rtpBuffer);
    producer.start();
    consumer.start();
    producer.wait();
    consumer.wait();
}

QTEST_APPLESS_MAIN(RtpTest)

#include "tst_rtptest.moc"
