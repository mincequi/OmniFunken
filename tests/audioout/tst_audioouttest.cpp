#include <QString>
#include <QtTest>
#include <QCoreApplication>

class AudioOutTest : public QObject
{
    Q_OBJECT

public:
    AudioOutTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void initAudioOut();
    void initAudioOut_data();
};

AudioOutTest::AudioOutTest()
{
}

void AudioOutTest::initTestCase()
{
}

void AudioOutTest::cleanupTestCase()
{
}

void AudioOutTest::initAudioOut()
{
    QFETCH(QString, data);
    QBENCHMARK {
    }
}

void AudioOutTest::initAudioOut_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

QTEST_MAIN(AudioOutTest)

#include "tst_audioouttest.moc"
