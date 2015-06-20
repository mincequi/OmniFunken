#include <ctime>

#include <QString>
#include <QThread>
#include <QtTest>
#include <QCoreApplication>

#include <audioout/audioout_ao.h>
#include <airtunes/airtunesconstants.h>
#include <rtp/rtpbuffer.h>
#include <rtp/rtpheader.h>
#include <rtp/rtppacket.h>

const uint numPacketsPerSecond = 44100/airtunes::framesPerPacket;
const quint16 initSeqNo = 65000;    // we want to wrap the buffer

class Producer : public QThread
{
public:
    Producer(RtpBuffer *rtpBuffer, uint interval) :
        m_rtpBuffer(rtpBuffer),
        m_interval(interval)
    {
    }

private:
    void run()
    {
        qDebug()<<Q_FUNC_INFO<< "start producing...";
        std::srand(std::time(0));
        for (quint16 i = initSeqNo; i != (quint16)(initSeqNo+(numPacketsPerSecond*10)); ++i) {
            RtpHeader header;
            header.sequenceNumber = i;

            QThread::msleep(m_interval);
            RtpPacket *rtpPacket = m_rtpBuffer->obtainPacket(header);
            for (uint j = 0; j < airtunes::framesPerPacket; ++j) {
                *(((int*)(rtpPacket->payload)+j)) = std::rand();
            }
            rtpPacket->payloadSize = airtunes::framesPerPacket*4;
            rtpPacket->sequenceNumber = i;
            m_rtpBuffer->commitPacket(rtpPacket);
        }
        qDebug()<<Q_FUNC_INFO<< "...end producing";
    }

    RtpBuffer *m_rtpBuffer;
    uint    m_interval;
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

        // New connect syntax not possible here because of default argument of Thread::start.
        connect(m_rtpBuffer, SIGNAL(ready()), this, SLOT(start()));
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
        while (true) {
            const RtpPacket *packet = m_rtpBuffer->takePacket();
            if (!packet) {
                qWarning()<<Q_FUNC_INFO<< "no packet from buffer. Stopping playback.";
                break;
            }
            m_audioOut->play(packet->payload, packet->payloadSize);
        } // while

        // Add silence after playback
        char *silence;
        int silenceSize;
        m_rtpBuffer->silence(&silence, &silenceSize);
        m_audioOut->play(silence, silenceSize);

        // Stop playback
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
    void slowProducer();
    void fastProducer();
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

void RtpTest::slowProducer()
{
    RtpBuffer *rtpBuffer= new RtpBuffer(airtunes::framesPerPacket, 500);
    Producer *producer = new Producer(rtpBuffer, 9);
    Consumer *consumer = new Consumer(rtpBuffer);

    producer->start();
    connect(producer, &Producer::finished, qApp, &QCoreApplication::quit);
    qApp->exec();
}

void RtpTest::fastProducer()
{
    RtpBuffer *rtpBuffer= new RtpBuffer(airtunes::framesPerPacket, 500);
    Producer *producer = new Producer(rtpBuffer, 6);
    Consumer *consumer = new Consumer(rtpBuffer);

    producer->start();
    connect(consumer, &Consumer::finished, qApp, &QCoreApplication::quit);
    qApp->exec();
}

QTEST_MAIN(RtpTest)

#include "tst_rtptest.moc"
