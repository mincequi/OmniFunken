#include "daemon.h"

#include <unistd.h>
#include <sys/stat.h>

#include <ao/ao.h>

#include <QDebug>

void init()
{
    // ao
    ao_initialize();
    int driver = ao_default_driver_id();
    ao_option *ao_opts = NULL;

    ao_sample_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.bits = 16;
    fmt.rate = 44100;
    fmt.channels = 2;
    fmt.byte_format = AO_FMT_NATIVE;

    ao_device *aodev = ao_open_live(driver, &fmt, ao_opts);
}

void daemon_init()
{
    pid_t pid = fork();
    if (pid < 0) {
        qFatal(__func__, ": fork error");
    } else if (pid > 0) {
        //qFatal(__func__, ": fork error");
        //init();
        exit(EXIT_SUCCESS);
    } else {
        init();
    }


    umask(0);

    if (setsid() < 0) {
        qFatal(__func__, ": setsid error");
    }

    if ((chdir("/")) < 0) {
        qFatal(__func__, ": chdir error");
    }

    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);
}
