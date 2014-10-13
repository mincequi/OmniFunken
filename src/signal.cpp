#include "signal.h"

#include <QCoreApplication>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

void handleSignal(int signo)
{
    switch(signo) {
    case SIGHUP:
        qDebug("hangup signal catched");
        break;
    case SIGINT:
        qWarning("interrupt signal catched");
        qApp->exit();
        break;
    case SIGTERM:
        qWarning("terminate signal catched");
        qApp->exit();
        break;
    }
}

void initSignalHandler()
{
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGINT,  handleSignal);
    signal(SIGHUP,  handleSignal); /* catch hangup signal */
    signal(SIGTERM, handleSignal); /* catch kill signal */
}

