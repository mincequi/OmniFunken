#include "rtspmessage.h"

#include <QRegExp>
#include <QTextStream>

RtspMessage::RtspMessage(QObject *parent) :
    QObject(parent),
    m_valid(true)
{
}

void RtspMessage::parse(const QByteArray &buffer)
{
    QRegExp rx("([A-Za-z-]+)\\:\\s(\\S*)\\r\\n");
    QString str(buffer);
    int pos = 0;
    while ((pos = rx.indexIn(str, pos)) != -1) {
        m_headers.insert(rx.cap(1).toLower().toLatin1(), rx.cap(2).toLatin1());
        pos += rx.matchedLength();
    }

    rx.setPattern("\\r\\n\\r\\n(.*)");
    rx.indexIn(str);
    m_body = rx.cap(1);
}

QByteArray RtspMessage::header(const QByteArray &key) const
{
    return m_headers.value(key.toLower(), "");
}

const QString & RtspMessage::body() const
{
    return m_body;
}

void RtspMessage::insert(const QByteArray &key, const QByteArray &value)
{
    m_headers.insert(key, value);
}

bool RtspMessage::valid() const
{
    return m_valid;
}

void RtspMessage::setValid(bool valid)
{
    m_valid = valid;
}

QByteArray RtspMessage::data()
{
    QByteArray data;
    QTextStream os(&data);

    os << "RTSP/1.0 200 OK\r\n";
    QMap<QByteArray, QByteArray>::iterator it;
    for (it = m_headers.begin(); it != m_headers.end(); ++it)
        os << it.key() << ": " << it.value() << "\r\n";
    os << "Audio-Jack-Status: connected; type=analog\r\n\r\n";
    //os << "\r\n";   // important!!!
    os.flush();
    return data;
}
