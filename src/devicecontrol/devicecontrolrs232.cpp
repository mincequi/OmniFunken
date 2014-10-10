#include "devicecontrolrs232.h"

#include "devicecontrolfactory.h"

#include <QDebug>


DeviceControlRs232::DeviceControlRs232() :
    m_serialPort(NULL),
    m_device("/dev/ttyUSB0"),
    m_baudRate(QSerialPort::UnknownBaud)
{
    DeviceControlFactory::registerDeviceControl(this);

    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
}

const char *DeviceControlRs232::name() const
{
    return "rs232";
}

bool DeviceControlRs232::init(const QString &device, const QString &group, QSettings *settings)
{
    m_device = device;

    settings->beginGroup(group);
    m_baudRate = settings->value("baud_rate", QSerialPort::Baud4800).toInt();
    QStringList keys = settings->childKeys();
    for (const QString &key : keys) {
        m_commands.insert(key, QByteArray::fromHex(settings->value(key).toString().remove(QRegExp("(0x|0X|x|X|\\s)")).toLatin1()));
    }
    settings->endGroup();

    /*
    m_serialPort.setPortName(m_device);
    if (!m_serialPort.open(QIODevice::WriteOnly)) {
        qWarning() << __func__ << ": unable to open device: " << m_device;
        return false;
    }
    if (!m_serialPort.setBaudRate(m_baudRate)) {
        qWarning() << __func__ << ": unable to set baud rate: " << m_baudRate;
        close();
        return false;
    }

    close();
    */
    return true;
}

void DeviceControlRs232::deinit()
{
    close();
}

void DeviceControlRs232::open()
{
    if (!m_serialPort.isOpen()) {
        m_serialPort.setPortName(m_device);
        m_serialPort.open(QIODevice::WriteOnly);
        m_serialPort.setBaudRate(m_baudRate);
        m_serialPort.setDataBits(QSerialPort::Data8);
        m_serialPort.setStopBits(QSerialPort::OneStop);
        m_serialPort.setParity(QSerialPort::NoParity);
        m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
    }
}

void DeviceControlRs232::close()
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }
}

void DeviceControlRs232::powerOn()
{
    if (m_commands.contains("power_on")) {
        write(m_commands.value("power_on"));
    }
}

void DeviceControlRs232::powerOff()
{
    if (m_commands.contains("power_off")) {
        write(m_commands.value("power_off"));
    }
}

void DeviceControlRs232::setInput()
{
    if (m_commands.contains("set_input")) {
        write(m_commands.value("set_input"));
    }
}

void DeviceControlRs232::setVolume(float volume)
{
    if (m_commands.contains("set_volume_prefix") || m_commands.contains("set_volume_suffix")) {
        write(getVolumeCommand(volume));
    }
}

void DeviceControlRs232::handleTimeout()
{
}

void DeviceControlRs232::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::WriteError) {
        qWarning() << __func__ << ": failed writing data";
    }
}

void DeviceControlRs232::write(const QByteArray &writeData)
{
    qint64 bytesWritten = m_serialPort.write(writeData);

    if (bytesWritten == -1) {
        qWarning() << __func__ << ": failed writing data";
    } else if (bytesWritten != writeData.size()) {
        qWarning() << __func__ << ": failed writing data";
    } else if (!m_serialPort.waitForBytesWritten(100)) {
        qWarning() << __func__ << ": timed out writing data";
    }
}

QByteArray DeviceControlRs232::getVolumeCommand(float volume)
{
    QByteArray command = m_commands.value("set_volume_prefix");
    if (volume <= -144.0f) {
        command.append((char)0x00);
    } else if (volume >= 0.0f) {
        command.append(m_commands.value("set_volume_max_value").at(0));
    } else {
        volume += 30.0f; // shift from 0.0 to 30.0
        volume /= 30.0f; // scale volume from 0.0 to 1.0
        command.append((m_commands.value("set_volume_max_value").at(0)*volume)+0.5f);
    }
    return command.append(m_commands.value("set_volume_suffix"));
}

static DeviceControlRs232 s_instance;
