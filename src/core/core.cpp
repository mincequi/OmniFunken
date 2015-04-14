#include "core.h"

#include "audioout/audioout_abstract.h"
#include "audioout/audiooutfactory.h"
#include "devicecontrol/devicecontrolabstract.h"
#include "devicecontrol/devicecontrolfactory.h"
#include "devicecontrol/devicewatcher.h"
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QMutex>
#include <QTimer>

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

CommandLineParseResult Core::parseCommandLine(QCommandLineParser &parser, QString *errorMessage)
{
    QCommandLineOption audioOutOption(QStringList() << "ao" << "audioout", "Set audio backend.", "audioout", "ao");
    parser.addOption(audioOutOption);
    QCommandLineOption audioDeviceOption(QStringList() << "ad" << "audiodevice", "Set audio device.", "audiodevice", "hw:0");
    parser.addOption(audioDeviceOption);

    if (!parser.parse(QCoreApplication::arguments())) {
        if (errorMessage) *errorMessage = parser.errorText();
        return CommandLineError;
    }

    m_audioOutName = parser.value(audioOutOption);
    m_audioDeviceName = parser.value(audioDeviceOption);

    qDebug() << ", audioOut: " << m_audioOutName
             << ", audioDevice: " << m_audioDeviceName;

    return CommandLineOk;
}

QSettings *Core::settings()
{
    return s_settings;
}

AudioOutAbstract *Core::audioOut()
{
    // Check if we have already an audio out device
    if (!m_audioOut) {
        // request configured audio out device
        QSettings::SettingsMap settings;
        m_audioOut = AudioOutFactory::createAudioOut(m_audioOutName, m_audioDeviceName, settings);
        if (!m_audioOut) {
            qWarning() << "AudioOut backend not available: " << m_audioOutName;
            return NULL;
        }
    }

    return m_audioOut;
}

DeviceControlAbstract *Core::deviceControl()
{
    return m_deviceControl;
}

void Core::powerOnDevice()
{
    // switch it on, if necessary
    if (m_audioOut->ready()) {
        return;
    }

    if (!m_deviceControl) {
        qDebug() << "No deviceControl available, cannot power on device.";
        return;
    }

    qDebug() << "AudioOut device not (yet) available: " << m_audioDeviceName;
    qDebug() << "trying to switch it on...";

    // open deviceControl, switch device on
    m_deviceControl->open();
    m_deviceControl->powerOn();

    // wait until available or time out
    DeviceWatcher *deviceWatcher = new DeviceWatcher();

    DeviceWatcher::UDevProperties properties;
    //properties["ID_MODEL"] = "Primare_I22_v1.0";
    settings()->beginGroup("device_watcher");
    QStringList keys = settings()->childKeys();
    for (const QString &key : keys) {
        if (key.compare("action", Qt::CaseInsensitive) == 0) continue;
        properties.insert(key, settings()->value(key).toString());
    }
    QString action = settings()->value("action", "add").toString();
    settings()->endGroup();

    QEventLoop loop;
    deviceWatcher->start(action, properties);
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, []() { qDebug() << "device ready"; });
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    QObject::connect(deviceWatcher, &DeviceWatcher::ready, deviceWatcher, &QObject::deleteLater);
    loop.exec();

    QSettings::SettingsMap settings;
    m_audioOut->init(settings);

    // select input
    m_deviceControl->setInput();
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

    // init device control
    m_deviceControl = DeviceControlFactory::createDeviceControl(s_settings);
}

Core::~Core()
{
    if (m_audioOut) {
        m_audioOut->deinit();
        m_audioOut = NULL;
    }
}
