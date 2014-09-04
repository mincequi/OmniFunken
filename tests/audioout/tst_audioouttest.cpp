#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <audioout/audiooutfactory.h>

class AudioOutTest : public QObject
{
    Q_OBJECT

public:
    AudioOutTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void audioOutStartStop();
    void audioOutPlay();

private:
    AudioOutAbstract *m_audioOut;
    QString m_driver;

};

AudioOutTest::AudioOutTest() :
    m_audioOut(NULL)
{
}

void AudioOutTest::initTestCase()
{
#ifdef Q_OS_MAC
    m_driver = "ao";
#elif
    m_driver = "alsa";
#endif

    QSettings::SettingsMap settings;
    m_audioOut = AudioOutFactory::createAudioOut(m_driver, settings);
}

void AudioOutTest::cleanupTestCase()
{
    m_audioOut->deinit();
}

void AudioOutTest::audioOutStartStop()
{
    QVERIFY(m_audioOut->name() == m_driver);
    QBENCHMARK {
        m_audioOut->start();
        m_audioOut->stop();
    }
}

void AudioOutTest::audioOutPlay()
{
    /*
    m_audioOut->start();
    m_audioOut->play();
    m_audioOut->stop();
    */
}


QTEST_MAIN(AudioOutTest)

#include "tst_audioouttest.moc"
