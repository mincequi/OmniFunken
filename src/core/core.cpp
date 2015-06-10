#include "core.h"

#include "audioout/audioout_abstract.h"
#include "audioout/audiooutfactory.h"
#include "devicecontrol/devicecontrolabstract.h"
#include "devicecontrol/devicecontrolfactory.h"
#include "devicecontrol/devicewatcher.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QHostInfo>
#include <QMutex>
#include <QTimer>
#include <QWaitCondition>

// The Core Singleton
static Core *s_instance = 0;
static QSettings *s_settings = 0;

Core *Core::instance()
{
    static QMutex mutex;
    if (!s_instance) {
        mutex.lock();
        if (!s_instance) {
            s_instance = new Core();
        }
        mutex.unlock();
    }
    return s_instance;
}

void Core::parseCommandLine(QCommandLineParser &parser)
{
    QString defaultName("OmniFunken@"); defaultName.append(QHostInfo::localHostName());
    QCommandLineOption nameOption(QStringList() << "n" << "name", "Set propagated name.", "name", defaultName);
    parser.addOption(nameOption);
    QCommandLineOption portOption(QStringList() << "p" << "port", "Set RTSP port.", "port", "5002");
    parser.addOption(portOption);
    QCommandLineOption latencyOption(QStringList() << "l" << "latency", "Set latency in milliseconds.", "latency", "500");
    parser.addOption(latencyOption);
    QCommandLineOption audioOutOption(QStringList() << "ao" << "audioout", "Set audio backend.", "audioout", "ao");
    parser.addOption(audioOutOption);
    QCommandLineOption audioDeviceOption(QStringList() << "ad" << "audiodevice", "Set audio device.", "audiodevice", "hw:0");
    parser.addOption(audioDeviceOption);

    parser.parse(QCoreApplication::arguments());

    m_options.name = parser.value(nameOption);
    m_options.port = parser.value(portOption).toInt();
    m_options.latency = parser.value(latencyOption).toInt();

    m_audioOutName = parser.value(audioOutOption);
    m_audioDeviceName = parser.value(audioDeviceOption);

    qDebug()<<Q_FUNC_INFO<<"name:"<<m_options.name<<"port:"<<m_options.port<<"latency:"<<m_options.latency;
    qDebug()<<Q_FUNC_INFO<<"audioOut:"<<m_audioOutName<<"audioDevice:"<<m_audioDeviceName;
}

const Core::Options &Core::options() const
{
    return m_options;
}

AudioOutAbstract *Core::audioOut()
{
    // Check if we have already an audio out device
    if (!m_audioOut) {
        // request registered audio out device
        m_audioOut = AudioOutFactory::createAudioOut(m_audioOutName);
        m_audioOut->setDevice(m_audioDeviceName);
        QSettings::SettingsMap settings;
        m_audioOut->init(settings);
    }

    // If device not ready, try to power it on
    if (!m_audioOut->ready()) {
        // This call blocks for a maximum of 5s
        if (!powerOnDevice(5000)) {
            return NULL;
        }
    }

    return m_audioOut;
}

DeviceControlAbstract *Core::deviceControl()
{
    if (m_deviceControl) {
        m_deviceControl->open();
    }
    return m_deviceControl;
}

bool Core::powerOnDevice(uint time)
{
    qDebug()<<Q_FUNC_INFO<<"enter";

    if (!deviceControl()) {
        qWarning()<<Q_FUNC_INFO<<"No deviceControl available, cannot power on device.";
        return false;
    }

    qDebug()<<Q_FUNC_INFO<<"AudioOut device not (yet) available:"<<m_audioDeviceName<<"trying to switch it on...";

    // open deviceControl, switch device on
    deviceControl()->powerOn();

    QMutex mutex;
    QWaitCondition deviceReady;

    // wait until available or time out
    DeviceWatcher *deviceWatcher = new DeviceWatcher();
    DeviceWatcher::UDevProperties properties;
    //properties["ID_MODEL"] = "Primare_I22_v1.0";
    s_settings->beginGroup("device_watcher");
    QStringList keys = s_settings->childKeys();
    for (const QString &key : keys) {
        if (key.compare("action", Qt::CaseInsensitive) == 0) continue;
        properties.insert(key, s_settings->value(key).toString());
    }
    QString action = s_settings->value("action", "add").toString();
    s_settings->endGroup();

    QEventLoop loop;
    deviceWatcher->start(action, properties);
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, []() { qDebug() << "device ready"; });
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, &loop, &QEventLoop::quit);
    QTimer::singleShot(time, &loop, SLOT(quit()));
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, deviceWatcher, &QObject::deleteLater);
    loop.exec();

    QSettings::SettingsMap settings;
    return m_audioOut->init(settings);

    // select input
    //m_deviceControl->setInput();
    //qDebug()<<Q_FUNC_INFO<<"exit";
}

bool Core::powerOnDevice2(uint time)
{
    if (!deviceControl()) {
        qWarning()<<Q_FUNC_INFO<<"No deviceControl available, cannot power on device.";
        return false;
    }

    qDebug()<<Q_FUNC_INFO<<"AudioOut device not (yet) available:"<<m_audioDeviceName<<"trying to switch it on...";

    // open deviceControl, switch device on
    deviceControl()->powerOn();

    QMutex mutex;
    QWaitCondition deviceReady;

    // wait until available or time out
    DeviceWatcher *deviceWatcher = new DeviceWatcher();
    DeviceWatcher::UDevProperties properties;
    //properties["ID_MODEL"] = "Primare_I22_v1.0";
    s_settings->beginGroup("device_watcher");
    QStringList keys = s_settings->childKeys();
    for (const QString &key : keys) {
        if (key.compare("action", Qt::CaseInsensitive) == 0) continue;
        properties.insert(key, s_settings->value(key).toString());
    }
    QString action = s_settings->value("action", "add").toString();
    s_settings->endGroup();

    QEventLoop loop;
    deviceWatcher->start(action, properties);
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, []() { qDebug() << "device ready"; });
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, &loop, &QEventLoop::quit);
    QTimer::singleShot(time, &loop, SLOT(quit()));
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, deviceWatcher, &QObject::deleteLater);
    loop.exec();

    QSettings::SettingsMap settings;
    m_audioOut->init(settings);

    // Returns true, if device is ready, false otherwise
    mutex.lock();
    return deviceReady.wait(&mutex, time);
}

void Core::shutdown()
{
    static QMutex mutex;
    mutex.lock();
    delete s_instance;
    s_instance = 0;
    mutex.unlock();
}

Core::Core() :
    m_audioOut(NULL),
    m_deviceControl(NULL)
{
    s_settings = new QSettings("/etc/omnifunken.conf", QSettings::IniFormat, this);

    m_deviceControl = DeviceControlFactory::createDeviceControl(s_settings);
}

Core::~Core()
{
    if (m_audioOut) {
        m_audioOut->deinit();
        m_audioOut = NULL;
    }

    if (m_deviceControl) {
        m_deviceControl->deinit();
        m_deviceControl = NULL;
    }
}
