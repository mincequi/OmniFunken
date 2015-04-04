#include "devicewatcher.h"

#include <QMap>
#include <libudev.h>
//#include <unistd.h>
#include <fcntl.h>

class Worker : public QObject
{
    Q_OBJECT
public slots:
    void doStart(const QString &action, const DeviceWatcher::UDevProperties &properties)
    {
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
            if (udev_device_get_action(dev) == action) {
                struct udev_list_entry *list_entry = udev_device_get_properties_list_entry(dev);

                // Iterate over our properties
                int numPropertiesFound = 0;
                for (const QString &key : properties.keys()) {
                    struct udev_list_entry *entry = udev_list_entry_get_by_name(list_entry, key.toLatin1());
                    if (!entry) continue;
                    if (udev_list_entry_get_value(entry) == properties.value(key)) {
                        numPropertiesFound++;
                    }
                }

                udev_device_unref(dev);

                if (numPropertiesFound == properties.size()) {
                    emit ready();
                    udev_unref(udev);
                    break;
                }
            }
        } // while
    }

signals:
    void ready();
};

DeviceWatcher::DeviceWatcher(QObject *parent) : QObject(parent)
{
    m_worker = new Worker();
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &Worker::ready, [this]() { m_started = false; emit ready(); });
    m_workerThread.start();
}

DeviceWatcher::~DeviceWatcher()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void DeviceWatcher::start(const QString &action, const UDevProperties &properties)
{
    if (m_started) return;

    m_started = true;
    QMetaObject::invokeMethod(m_worker, "doStart", Qt::QueuedConnection,
                              Q_ARG(QString, action),
                              Q_ARG(UDevProperties, properties));
}
