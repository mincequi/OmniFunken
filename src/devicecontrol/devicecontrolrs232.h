#ifndef DEVICECONTROLRS232_H
#define DEVICECONTROLRS232_H

#include "devicecontrolabstract.h"

#include <QtSerialPort/QtSerialPort>
#include <QTextStream>
#include <QTimer>

class DeviceControlRs232 : public DeviceControlAbstract
{
    Q_OBJECT
public:
    DeviceControlRs232();

    virtual const char *name() const override;
    virtual bool init(const QString &device, const QString &group, QSettings *settings) override;
    virtual void deinit() override;
    virtual void open() override;
    virtual void close() override;
    virtual void powerOn() override;
    virtual void powerOff() override;
    virtual void setInput() override;
    virtual void setVolume(float volume) override;

private slots:
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);

private:
    void write(const QByteArray &writeData);
    QByteArray getVolumeCommand(float volume);

    QSerialPort m_serialPort;
    QString     m_device;
    qint32      m_baudRate;

    QMap<QString, QByteArray> m_commands;
};

#endif // DEVICECONTROLRS232_H
