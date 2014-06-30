#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include <QObject>

class RtpReceiver : public QObject
{
    Q_OBJECT
public:
    enum PayloadType {
        TimingRequest       = 82,
        TimingResponse      = 83,
        Sync                = 84,
        RetransmitRequest   = 85,
        RetransmitResponse  = 86,
        AudioData           = 96
    };

    explicit RtpReceiver(QObject *parent = 0);

signals:
    //void socketRequired();

public slots:
    void setup();

};

#endif // RTPRECEIVER_H
