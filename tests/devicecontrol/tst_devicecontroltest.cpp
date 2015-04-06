#include <QString>
#include <QtTest>

#include <devicecontrol/devicecontrolabstract.h>
#include <devicecontrol/devicecontrolfactory.h>
#include <devicecontrol/devicewatcher.h>

class DeviceControlTest : public QObject
{
    Q_OBJECT

public:
    DeviceControlTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void powerOn();
    void powerOff();

    void watchDevice();

private:
    DeviceControlAbstract   *m_deviceControl;
    DeviceWatcher           *m_deviceWatcher;
};

DeviceControlTest::DeviceControlTest() :
    m_deviceControl(NULL),
    m_deviceWatcher(new DeviceWatcher(this))
{
}

void DeviceControlTest::initTestCase()
{
    // TODO: rework
    /*
    QSettings::SettingsMap settings;
    settings.insert("power_on", "0x02 0x57 0x81 0x01 0x10 0x03");
    settings.insert("power_off", "0x02 0x57 0x81 0x00 0x10 0x03");
    settings.insert("set_input", "0x02 0x57 0x82 0xYY 0x10 0x03"); //#YY= 0x01 to 0x07
    settings.insert("set_volume", "0x02 0x57 0x83 0xYY 0x10 0x03"); //#YY=0x00 to 0x4F
    settings.insert("min_volume", "0x00");
    settings.insert("max_volume", "0x30");

    m_deviceControl = DeviceControlFactory::createDeviceControl(settings);
    QVERIFY(m_deviceControl->name() == "rs232");

    bool ok = m_deviceControl->init(settings);
    m_deviceControl->open();
    */
}

void DeviceControlTest::cleanupTestCase()
{
    //m_deviceControl->close();
}

void DeviceControlTest::powerOn()
{
    //m_deviceControl->powerOn();
}

void DeviceControlTest::powerOff()
{
}

void DeviceControlTest::watchDevice()
{
    DeviceWatcher::UDevProperties properties;
    properties["ID_MODEL"] = "Primare_I22_v1.0";
    connect(m_deviceWatcher, &DeviceWatcher::ready, []() { qDebug() << "device ready"; });
    m_deviceWatcher->start("add", properties);

    QEventLoop loop;
    connect(m_deviceWatcher, &DeviceWatcher::ready, &loop, &QEventLoop::quit);
    loop.exec();
}

QTEST_MAIN(DeviceControlTest)

#include "tst_devicecontroltest.moc"
