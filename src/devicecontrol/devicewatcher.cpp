#include "devicewatcher.h"

#ifdef Q_OS_LINUX
#include <libudev.h>
#include <fcntl.h>
#endif

DeviceWatcher::DeviceWatcher(QObject *parent) :
    QObject(parent),
    m_started(false)
{
}

DeviceWatcher::~DeviceWatcher()
{
}

void DeviceWatcher::start(const QString &action, const UDevProperties &properties)
{
    if (m_started) return;

    m_started = true;

    WorkerThread *workerThread = new WorkerThread(this);
    workerThread->init(action, properties);
    connect(workerThread, &WorkerThread::ready, [this]() { m_started = false; emit ready(); });
    connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
    workerThread->start();
}

WorkerThread::WorkerThread(QObject *parent) :
    QThread(parent)
{
}

void WorkerThread::init(const QString &action, const DeviceWatcher::UDevProperties &properties)
{
    m_action = action;
    m_properties = properties;
}

void WorkerThread::run()
{
#ifdef Q_OS_LINUX
    struct udev *udev = udev_new();
    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(mon, "sound", NULL);
    udev_monitor_enable_receiving(mon);

    // Get the file descriptor (fd) for the monitor.
    int fd = udev_monitor_get_fd(mon);
    // Set file descriptor to blocking mode.
    int flags = fcntl(fd, F_GETFL);
    // Set the new flags with O_NONBLOCK masked out
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    while (1) {
        // Call to receive the device. The monitor socket is by default set to NONBLOCK!
        struct udev_device *dev = udev_monitor_receive_device(mon);
        if (!dev) continue;

        // Check for our action.
        if (udev_device_get_action(dev) == m_action) {
            struct udev_list_entry *list_entry = udev_device_get_properties_list_entry(dev);

            // Iterate over our properties
            int numPropertiesFound = 0;
            for (const QString &key : m_properties.keys()) {
                struct udev_list_entry *entry = udev_list_entry_get_by_name(list_entry, key.toLatin1());
                if (!entry) continue;
                if (udev_list_entry_get_value(entry) == m_properties.value(key)) {
                    numPropertiesFound++;
                }
            }

            udev_device_unref(dev);

            if (numPropertiesFound == m_properties.size()) {
                emit ready();
                udev_unref(udev);
                break;
            }
        }
    } // while
#endif
}

