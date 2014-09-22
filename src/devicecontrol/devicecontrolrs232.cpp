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
    return true;
}

void DeviceControlRs232::deinit()
{
}

void DeviceControlRs232::open()
{
    m_serialPort.setPortName(m_portName);
    m_serialPort.setBaudRate(m_baudRate);
    m_serialPort.open(QIODevice::WriteOnly);
}

void DeviceControlRs232::close()
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }
}

void DeviceControlRs232::powerOn()
{
    write(QByteArray::fromHex("025781011003"));
}

void DeviceControlRs232::powerOff()
{
}

void DeviceControlRs232::setInput()
{
}

void DeviceControlRs232::setVolume(float volume)
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
    //m_writeData = writeData;

    qint64 bytesWritten = m_serialPort.write(writeData);

    if (bytesWritten != writeData.size()) {
        qWarning() << __PRETTY_FUNCTION__ << ": failed writing data";
        return;
    }
}

static DeviceControlRs232 s_instance;
