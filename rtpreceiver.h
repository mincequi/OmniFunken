#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include <QObject>

class RtpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit RtpReceiver(QObject *parent = 0);

signals:
    void newSocket();

public slots:
    //void

};

#endif // RTPRECEIVER_H
