#include "devicewatcher.h"

#include <libudev.h>

devicewatcher::devicewatcher(QObject *parent) : QObject(parent)
{
}

devicewatcher::~devicewatcher()
{

}

void devicewatcher::start()
{
    struct udev *udev = udev_new();
    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(mon, "sound", NULL);
    udev_monitor_enable_receiving(mon);

#ifdef USE_SELECT
    // Get the file descriptor (fd) for the monitor.
    fd = udev_monitor_get_fd(mon);

    // We use select() system call to not use blocking udev_monitor_receive_device() call.
    while (1) {
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        // Timveval set to 0 will result in non.blocking select() operation.
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        // Check if our file descriptor has received data.
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            printf("\nselect() says there should be data\n");
#else
    while (true) {
#endif
        // Call to receive the device. This will block.
        dev = udev_monitor_receive_device(mon);
        if (dev) {
            printf("Got Device\n");
            printf("   Node: %s\n", udev_device_get_devnode(dev));
            printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
            printf("   Devtype: %s\n", udev_device_get_devtype(dev));
            printf("   Action: %s\n", udev_device_get_action(dev));

            struct udev_list_entry *list_entry = udev_device_get_properties_list_entry(dev);
            struct udev_list_entry* model_entry = udev_list_entry_get_by_name(list_entry, "ID_MODEL");

            if (0 != model_entry) {
                const char* szModelValue = udev_list_entry_get_value(model_entry);
                printf("   Model: %s\n", szModelValue);
            }

            udev_device_unref(dev);
        } else {
            printf("No Device from receive_device(). An error occured.\n");
        }
    }
#ifdef USE_SELECT
        usleep(250*1000);
        printf(".");
        fflush(stdout);
    } // while
#endif

    udev_unref(udev);
}


