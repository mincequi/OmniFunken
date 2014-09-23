#include "devicecontrolrs232.h"

#include "devicecontrolfactory.h"

#include <QDebug>

DeviceControlRs232::DeviceControlRs232() :
    m_serialPort(NULL),
    m_portName("/dev/ttyUSB0"),
    m_baudRate(QSerialPort::UnknownBaud)
{
    DeviceControlFactory::registerDeviceControl(this);

    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
}

const char *DeviceControlRs232::name() const
{
    return "rs232";
}

bool DeviceControlRs232::init(const QSettings::SettingsMap &settings)
{
    m_baudRate = QSerialPort::Baud4800;

    auto it = settings.constBegin();
    while (it != settings.constEnd()) {
        m_commands.insert(it.key(), it.value().toString().remove(QRegExp("(0x|0X|x|X|\\s)")).toLatin1());
        ++it;
    }

    return true;
}

void DeviceControlRs232::deinit()
{
}

void DeviceControlRs232::open()
{
    m_serialPort.setPortName(m_portName);
    m_serialPort.open(QIODevice::WriteOnly);
    m_serialPort.setBaudRate(m_baudRate);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
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
        write(QByteArray::fromHex(m_commands.value("power_on")));
    }
}

void DeviceControlRs232::powerOff()
{
    if (m_commands.contains("power_off")) {
        write(QByteArray::fromHex(m_commands.value("power_off")));
    }
}

void DeviceControlRs232::setInput()
{
    if (m_commands.contains("set_input")) {
        write(QByteArray::fromHex(m_commands.value("set_input")));
    }
}

void DeviceControlRs232::setVolume(float volume)
{
    if (m_commands.contains("set_volume")) {
        write(getVolumeCommand(volume));
    }
}

void DeviceControlRs232::handleTimeout()
{
}

void DeviceControlRs232::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::WriteError) {
        qWarning() << __PRETTY_FUNCTION__ << ": failed writing data";
    }
}

void DeviceControlRs232::write(const QByteArray &writeData)
{
    qint64 bytesWritten = m_serialPort.write(writeData);
    m_serialPort.waitForBytesWritten(100);
}

QByteArray DeviceControlRs232::getVolumeCommand(float volume)
{
    QByteArray command = m_commands.value("set_volume");
    if (volume <= -144.0f) {
        return command.replace("YY", "00");
    } else if (volume >= 0.0f) {
        return command.replace("YY", m_commands.value("max_volume"));
    } else {
        volume += 30.0f; // shift from 0.0 to 30.0
        volume /= 30.0f; // scale volume from 0.0 to 1.0
        bool ok = false;
        QByteArray number = QByteArray::number((uint)((m_commands.value("max_volume").toUInt(&ok, 16)*volume)+0.5f), 16);
        if (number.size() == 1) {
            number.prepend("0");
        }
        return command.replace("YY", number);
    }
}

static DeviceControlRs232 s_instance;
