#include "core.h"

#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QMutex>

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

QSettings *Core::settings()
{
    return s_settings;
}

void Core::shutdown()
{
    static QMutex mutex;
    mutex.lock();
    delete s_instance;
    s_instance = 0;
    mutex.unlock();
}

Core::Core()
{
    s_settings = new QSettings("/etc/omnifunken.conf", QSettings::IniFormat, this);
}

Core::~Core()
{
}
