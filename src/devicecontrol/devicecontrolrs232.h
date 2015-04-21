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

    virtual const char *name() const Q_DECL_OVERRIDE;
    virtual bool init(const QString &device, const QString &group, QSettings *settings) Q_DECL_OVERRIDE;
    virtual void deinit() Q_DECL_OVERRIDE;
    virtual void open() Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;
    virtual void powerOn() Q_DECL_OVERRIDE;
    virtual void powerOff() Q_DECL_OVERRIDE;
    virtual void setInput() Q_DECL_OVERRIDE;
    virtual void setVolume(float volume) Q_DECL_OVERRIDE;

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
